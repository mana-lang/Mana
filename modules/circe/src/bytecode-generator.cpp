#include <circe/core/logger.hpp>
#include <circe/bytecode-generator.hpp>

#include <sigil/ast/keywords.hpp>
#include <sigil/ast/semantic-analyzer.hpp>

#include <hexe/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <ranges>

namespace circe {
using namespace hexe;
using namespace sigil::ast;

BytecodeGenerator::BytecodeGenerator()
    : scope {0},
      bytecode {} {}

ByteCode BytecodeGenerator::Bytecode() const {
    return bytecode;
}

void BytecodeGenerator::ObtainSemanticAnalysisInfo(const sigil::SemanticAnalyzer& analyzer) {
    for (const auto name : analyzer.Globals() | std::views::keys) {
        AddSymbol(name, global_registers.Allocate());
        global_registers.Lock(symbols[name].register_index);
    }

    for (const auto& info : analyzer.Types() | std::views::values) {
        for (const auto& [name, in_func] : info.functions) {
            auto& fn = functions[name];

            fn.return_type = in_func.return_type;
            fn.registers.Reserve(in_func.locals.size());
        }
    }
}

void BytecodeGenerator::Visit(const Artifact& artifact) {
    for (const auto& decl : artifact.GetChildren()) {
        decl->Accept(*this);
    }

    for (const auto& [index, name] : pending_calls) {
        if (not functions.contains(name)) {
            Log->error("Internal Compiler Error: Attempted to call non-existent function '{}'", name);
            continue;
        }

        const auto address = functions.at(name).address;
        if (address < 0) {
            Log->error("Internal Compiler Error: Attempted to call unresolved function '{}'", name);
            continue;
        }

        bytecode.PatchCall(index, address);
    }

    bytecode.SetMainRegisterFrame(global_registers.Total());
}

void BytecodeGenerator::Visit(const Scope& node) {
    EnterScope();

    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);

        Registers().Free(register_buffer);
        register_buffer.clear();
    }

    ExitScope();
}

void BytecodeGenerator::Visit(const FunctionDeclaration& node) {
    const auto& name   = node.GetName();
    const auto& params = node.GetParameters();
    const auto& body   = node.GetBody();

    auto& fn = functions[name];

    fn.return_type = node.GetReturnType();
    fn.address     = bytecode.CurrentAddress();

    ++scope;
    {
        const auto param_regs = fn.registers.ViewLocked();
        for (i64 i = 0; i < params.size(); ++i) {
            AddSymbol(params[i].name, param_regs[i]);
        }
    }
    --scope;

    function_stack.push_back(name);
    body->Accept(*this);
    function_stack.pop_back();


    if (sigil::IsEntryPoint(name)) {
        bytecode.SetEntryPoint(fn.address);

        // main doesn't return; it halts
        bytecode.Write(Op::Halt);
        return;
    }
    // functions that return nothing return automatically at the end of their scope
    using enum sigil::PrimitiveType;
    if (fn.return_type == PrimitiveName(None) && bytecode.LatestOpcode() != Op::Return) {
        ReturnNone();
    }
}

void BytecodeGenerator::Visit(const MutableDataDeclaration& node) {
    HandleInitializer(node, true);
}

void BytecodeGenerator::Visit(const DataDeclaration& node) {
    HandleInitializer(node, false);
}

void BytecodeGenerator::Visit(const Identifier& node) {
    // even though input is assumed to be correct
    // we don't wanna fail silently if semantic analysis fails
    if (const auto it = symbols.find(node.GetName());
        it != symbols.end()) {
        register_buffer.push_back(it->second.register_index);
    }
}

void BytecodeGenerator::Visit(const Assignment& node) {
    const auto& symbol = symbols[node.GetIdentifier()];

    node.GetValue()->Accept(*this);

    const auto rhs = PopRegBuffer();
    const auto lhs = symbol.register_index;

    const auto op = node.GetOp();
    if (op == "=") {
        bytecode.Write(Op::Move, {lhs, rhs});
    } else {
        auto operation = Op::Err;
        switch (op[0]) {
        case '+':
            operation = Op::Add;
            break;
        case '-':
            operation = Op::Sub;
            break;
        case '*':
            operation = Op::Mul;
            break;
        case '/':
            operation = Op::Div;
            break;
        case '%':
            operation = Op::Mod;
            break;
        default:
            break;
        }

        bytecode.Write(operation, {lhs, lhs, rhs});
    }

    Registers().Free(rhs);
}

void BytecodeGenerator::Visit(const Return& node) {
    if (sigil::IsEntryPoint(CurrentFunctionName())) {
        bytecode.Write(Op::Halt);
        return;
    }

    const auto& return_expr = node.GetExpression();
    if (return_expr != nullptr) {
        return_expr->Accept(*this);
        const auto return_value = PopRegBuffer();

        bytecode.Write(Op::Return, {return_value});
        Registers().Free(return_value);
    } else {
        // functions that return 'none' don't affect the return register at all
        ReturnNone();
    }
}

void BytecodeGenerator::Visit(const Invocation& node) {
    const auto name  = node.GetIdentifier();
    const auto& args = node.GetArguments();
    const auto& fn   = functions[name];

    // handle print
    if (name == "Print") {
        args[0]->Accept(*this);
        bytecode.Write(Op::Print, {PopRegBuffer()});

        register_buffer.push_back(REGISTER_RETURN);
        return;
    }

    if (name == "PrintV") {
        args[0]->Accept(*this);
        const auto str_reg = PopRegBuffer();
        args[1]->Accept(*this);
        const auto val = PopRegBuffer();

        bytecode.Write(Op::PrintValue, {str_reg, val});

        register_buffer.push_back(REGISTER_RETURN);
        return;
    }

    HandleInvocationArguments(node.GetArguments(), fn.registers.ViewLocked());

    if (fn.address == bytecode.CurrentAddress()) {
        Log->error("Internal Compiler Error: Invocation {} jumps to call site '{}'", name, fn.address);
        return;
    }

    if (fn.address < 0) {
        const auto index     = bytecode.WriteCall(SENTINEL_32, Registers().Total());
        pending_calls[index] = name;
    } else {
        bytecode.WriteCall(fn.address, Registers().Total());
    }

    register_buffer.push_back(REGISTER_RETURN); // functions always return something
}

void BytecodeGenerator::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const auto cond_reg = PopRegBuffer();

    const i64 jmp_false = bytecode.Write(Op::JumpWhenFalse, {cond_reg, SENTINEL});
    Registers().Free(cond_reg);

    node.GetThenBlock()->Accept(*this);

    if (const auto& else_branch = node.GetElseBranch()) {
        const i64 jmp_end = bytecode.Write(Op::Jump, {SENTINEL});
        PatchJumpForwardConditional(jmp_false);

        else_branch->Accept(*this);
        PatchJumpForward(jmp_end);
    } else {
        PatchJumpForwardConditional(jmp_false);
    }
}

void BytecodeGenerator::Visit(const Loop& node) {
    EnterLoop();

    const i64 start_addr = bytecode.CurrentAddress();

    node.GetBody()->Accept(*this);

    // calc skips before loop ends so we don't jump over them
    // this also helps resolve end-of-loop logic in other types of loops
    // in here, we just do it this way for consistency
    HandlePendingSkips();

    // end of loop
    JumpBackwards(start_addr);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopIf& node) {
    EnterLoop();

    const i64 start_addr = bytecode.CurrentAddress();

    node.GetCondition()->Accept(*this);
    const auto condition = PopRegBuffer();

    // the jump out of the loop needs to happen immediately after condition evaluation
    const i64 jmp_end = bytecode.Write(Op::JumpWhenFalse, {condition, SENTINEL});
    Registers().Free(condition);

    node.GetBody()->Accept(*this);
    HandlePendingSkips();

    JumpBackwards(start_addr);
    PatchJumpForwardConditional(jmp_end);

    HandlePendingBreaks();

    ExitLoop();
}

// same as LoopIf, except we do the condition evaluation after the body
void BytecodeGenerator::Visit(const LoopIfPost& node) {
    EnterLoop();

    const i64 start_addr = bytecode.CurrentAddress();

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    // end of loop
    node.GetCondition()->Accept(*this);
    const auto condition = PopRegBuffer();
    JumpBackwardsConditional(Op::JumpWhenTrue, condition, start_addr);

    Registers().Free(condition);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopRange& node) {
    EnterLoop();

    const auto range = PerformRangeLoopSetup(node);

    // we add or subtract 1 since ranges are inclusive
    // this lets us compare to "equals" rather than jwf greater/lesser
    bytecode.Write(Op::Add, {range.end, range.end, range.step});

    const auto cond = Registers().Allocate();

    // loop starts here
    const i64 start_addr = bytecode.CurrentAddress();

    bytecode.Write(Op::Equals, {cond, range.counter, range.end});
    const i64 exit = bytecode.Write(Op::JumpWhenTrue, {cond, SENTINEL});

    node.GetBody()->Accept(*this);
    HandlePendingSkips();

    bytecode.Write(Op::Add, {range.counter, range.counter, range.step});
    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    Registers().Free(cond);
    Registers().Free(range.counter);
    Registers().Free(range.step);
    Registers().Free(range.end);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopRangeMutable& node) {
    EnterLoop();

    const auto range = PerformRangeLoopSetup(node);

    // allocate persistent registers before loop
    const auto zero = Registers().Allocate();
    bytecode.Write(Op::LoadConstant, {zero, bytecode.AddConstant(0)});

    const auto cond = Registers().Allocate();

    // loop starts here
    const i64 start_addr = bytecode.CurrentAddress();

    // for mutable loop counters, we can't just compare to equals
    // so we do something slightly more involved
    // (end - counter) * step >= 0
    const auto diff = Registers().Allocate();
    bytecode.Write(Op::Sub, {diff, range.end, range.counter});
    bytecode.Write(Op::Mul, {diff, diff, range.step});

    bytecode.Write(Op::Cmp_GreaterEq, {cond, diff, zero});
    const i64 exit = bytecode.Write(Op::JumpWhenFalse, {cond, SENTINEL});

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    bytecode.Write(Op::Add, {range.counter, range.counter, range.step});

    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    Registers().Free(range.end);
    Registers().Free(range.step);
    Registers().Free(range.counter);
    Registers().Free(cond);
    Registers().Free(diff);
    Registers().Free(zero);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopFixed& node) {
    EnterLoop();

    const auto counter = Registers().Allocate();
    bytecode.Write(Op::LoadConstant, {counter, bytecode.AddConstant(0)});

    const auto step = Registers().Allocate();
    bytecode.Write(Op::LoadConstant, {step, bytecode.AddConstant(1)});

    // we haven't entered the body yet, but the target belongs to that scope
    // since the target may be a literal, we need to do it like this
    ++scope;
    node.GetCountTarget()->Accept(*this);
    const auto target = PopRegBuffer();
    --scope;

    const i64 start_addr = bytecode.CurrentAddress();

    // while the parser tries to guard against negative counts, they may be undetectable at compile time
    // in that case, the loop would end immediately
    const auto cond = Registers().Allocate();
    bytecode.Write(Op::Cmp_Lesser, {cond, counter, target});
    const i64 exit = bytecode.Write(Op::JumpWhenFalse, {cond, SENTINEL});

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    // increment and bounce back to start
    bytecode.Write(Op::Add, {counter, counter, step});

    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    HandlePendingBreaks();

    Registers().Free(cond);
    Registers().Free(target);
    Registers().Free(step);
    Registers().Free(counter);

    ExitLoop();
}

void BytecodeGenerator::Visit(const Break& node) {
    HandleLoopControl(true, node.GetCondition());
}

void BytecodeGenerator::Visit(const Skip& node) {
    HandleLoopControl(false, node.GetCondition());
}

void BytecodeGenerator::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);
    const auto op_text = node.GetOp();

    auto src = PopRegBuffer();
    auto dst = Registers().Allocate();

    Op op;
    switch (op_text[0]) {
    case '-':
        op = Op::Negate;
        break;
    case 'n': // 'not'
        [[fallthrough]];
    case '!':
        op = Op::Not;
        break;
    default:
        Log->error("Internal Compiler Error: Invalid unary expression");
        Registers().Free(src);
        return;
    }

    bytecode.Write(op, {dst, src});
    register_buffer.push_back(dst);
    Registers().Free(src);
}

void BytecodeGenerator::Visit(const BinaryExpr& node) {
    const auto op_text = node.GetOp();

    // logical ops need special treatment as they are control flow due to short-circuiting
    const auto jump = [this, &node](const Op jump_op) {
        node.GetLeft().Accept(*this);

        const auto lhs = PopRegBuffer();
        const auto dst = Registers().Allocate();
        bytecode.Write(Op::Move, {dst, lhs});

        const i64 jwf = bytecode.Write(jump_op, {lhs, SENTINEL});
        Registers().Free(lhs);

        node.GetRight().Accept(*this);
        const auto rhs = PopRegBuffer();
        bytecode.Write(Op::Move, {dst, rhs});

        PatchJumpForwardConditional(jwf);

        register_buffer.push_back(dst);
        Registers().Free(rhs);
    };

    if (op_text == "&&") {
        jump(Op::JumpWhenFalse);
        return;
    }

    if (op_text == "||") {
        jump(Op::JumpWhenTrue);
        return;
    }

    node.GetLeft().Accept(*this);
    node.GetRight().Accept(*this);

    auto rhs = PopRegBuffer();
    auto lhs = PopRegBuffer();
    auto dst = Registers().Allocate();

    Op op;
    switch (op_text[0]) {
    case '+':
        op = Op::Add;
        break;
    case '-':
        op = Op::Sub;
        break;
    case '*':
        op = Op::Mul;
        break;
    case '/':
        op = Op::Div;
        break;
    case '%':
        op = Op::Mod;
        break;
    case '>':
        if (op_text == ">=") {
            op = Op::Cmp_GreaterEq;
            break;
        }
        op = Op::Cmp_Greater;
        break;
    case '<':
        if (op_text == "<=") {
            op = Op::Cmp_LesserEq;
            break;
        }
        op = Op::Cmp_Lesser;
        break;
    case '=':
        if (op_text == "==") {
            op = Op::Equals;
            break;
        }
    case '!':
        if (op_text == "!=") {
            op = Op::NotEquals;
            break;
        }
    default:
        Log->error("Internal Compiler Error: Unknown Binary Operator '{}'", node.GetOp());
        return;
    }

    bytecode.Write(op, {dst, lhs, rhs});
    register_buffer.push_back(dst);
    Registers().Free({lhs, rhs});
}

// we'll add arrays eventually
void BytecodeGenerator::Visit(const ListLiteral& array) {
    std::unreachable();
}

void BytecodeGenerator::Visit(const Literal<f64>& float64) {
    CreateLiteral(float64.Get());
}

void BytecodeGenerator::Visit(const Literal<i64>& int64) {
    CreateLiteral(int64.Get());
}

void BytecodeGenerator::Visit(const Literal<bool>& boolean) {
    CreateLiteral(boolean.Get());
}

void BytecodeGenerator::Visit(const StringLiteral& string) {
    CreateLiteral(string.Get());
}

bool BytecodeGenerator::IsConditionalJumpOp(const Op op) const {
    return op == Op::JumpWhenTrue || op == Op::JumpWhenFalse;
}

void BytecodeGenerator::JumpBackwards(i64 target_index) {
    bytecode.Write(Op::Jump, {CalcJump(target_index, false, false)});
}

void BytecodeGenerator::JumpForward(i64 target_index) {
    bytecode.Write(Op::Jump, {CalcJump(target_index, true, false)});
}

void BytecodeGenerator::JumpBackwardsConditional(const Op op, Register condition_register, i64 target_index) {
    if (not IsConditionalJumpOp(op)) {
        Log->error("Internal Compiler Error: JumpBackwardsConditional called on non-jump op '{}'",
                   magic_enum::enum_name(op)
        );
        return;
    }
    bytecode.Write(op, {condition_register, CalcJump(target_index, false, true)});
}

void BytecodeGenerator::PatchJumpForward(i64 target_index) {
    bytecode.Patch(target_index, CalcJump(target_index, true, false));
}

void BytecodeGenerator::PatchJumpBackward(i64 target_index) {
    bytecode.Patch(target_index, CalcJump(target_index, false, false));
}

void BytecodeGenerator::PatchJumpForwardConditional(i64 target_index) {
    static constexpr u8 conditional_offset = 1;
    bytecode.Patch(target_index, CalcJump(target_index, true, true), conditional_offset);
}

void BytecodeGenerator::PatchJumpBackwardConditional(i64 target_index) {
    static constexpr u8 conditional_offset = 1;
    bytecode.Patch(target_index, CalcJump(target_index, false, true), conditional_offset);
}

bool JumpIsWithinBounds(const i64 distance) {
    return distance <= std::numeric_limits<i16>::max()
           && distance >= std::numeric_limits<i16>::min();
}

Register BytecodeGenerator::CalcJump(const i64 target_index, bool is_forward, bool is_conditional) const {
    const auto jump_bytes = is_conditional ? CJMP_OP_BYTES : JMP_OP_BYTES;

    const i64 jump_distance = is_forward
                                  ? bytecode.CurrentAddress() - (target_index + jump_bytes)
                                  : target_index - (bytecode.CurrentAddress() + jump_bytes);

    if (not JumpIsWithinBounds(jump_distance)) {
        Log->error("Internal Compiler Error: Jump distance out of bounds");
        return SENTINEL;
    }

    return static_cast<Register>(jump_distance);
}

RegisterFrame& BytecodeGenerator::Registers() {
    if (function_stack.empty()) {
        return global_registers;
    }

    return CurrentFunction().registers;
}

Register BytecodeGenerator::PopRegBuffer() {
    if (register_buffer.empty()) {
        Log->error("Internal Compiler Error: Register stack underflow");
        return -1;
    }

    const auto slot = register_buffer.back();
    register_buffer.pop_back();

    return slot;
}

const BytecodeGenerator::Function& BytecodeGenerator::CurrentFunction() const {
    return functions.at(function_stack.back());
}

BytecodeGenerator::Function& BytecodeGenerator::CurrentFunction() {
    return functions.at(function_stack.back());
}

std::string_view BytecodeGenerator::CurrentFunctionName() const {
    return function_stack.back();
}

void BytecodeGenerator::HandleInvocationArguments(std::span<const NodePtr> args, std::span<const Register> param_regs) {
    std::vector<Register> arg_regs;
    arg_regs.reserve(args.size());

    for (const auto& arg : args) {
        arg->Accept(*this);
        arg_regs.push_back(PopRegBuffer());
    }

    for (i64 i = 0; i < arg_regs.size(); ++i) {
        const u8 dst = param_regs[i] + Registers().Total();
        bytecode.Write(Op::Move, {dst, arg_regs[i]});
    }

    Registers().Free(arg_regs);
}

void BytecodeGenerator::ReturnNone() {
    bytecode.Write(Op::Return, {REGISTER_RETURN});
}

void BytecodeGenerator::EnterScope() {
    ++scope;
}

void BytecodeGenerator::ExitScope() {
    std::vector<std::string_view> deleted_symbols;
    deleted_symbols.reserve(symbols.size());

    for (const auto& [name, symbol] : symbols) {
        if (symbol.scope == scope) {
            deleted_symbols.push_back(name);
        }
    }

    for (const auto& name : deleted_symbols) {
        RemoveSymbol(name);
    }

    --scope;
}

void BytecodeGenerator::EnterLoop() {
    loop_stack.emplace_back();
}

void BytecodeGenerator::ExitLoop() {
    loop_stack.pop_back();
}

void BytecodeGenerator::AddSymbol(const std::string_view name, const Register index) {
    // as it stands, duplicate symbol names across different functions get clobbered
    // but this isn't a big deal because the BCG only operates function-locally anyway
    symbols[name] = {index, scope};
}

void BytecodeGenerator::RemoveSymbol(const std::string_view name) {
    if (not symbols.contains(name)) {
        Log->warn("Internal Compiler Error: Attempted to remove non-existent symbol '{}'", name);
        return;
    }

    // prevent duplicate entries in e.g. 'data x = y'
    const auto reg = symbols[name].register_index;
    symbols.erase(name);
    Registers().Free(reg);
}

BytecodeGenerator::LoopContext& BytecodeGenerator::CurrentLoop() {
    return loop_stack.back();
}

BytecodeGenerator::RangeLoopRegisters BytecodeGenerator::PerformRangeLoopSetup(const LoopRange& node) {
    const auto alloc_origin = [this](const NodePtr& origin) {
        if (origin == nullptr) {
            const auto reg = Registers().Allocate();
            bytecode.Write(Op::LoadConstant, {reg, bytecode.AddConstant(0)});
            return reg;
        }

        origin->Accept(*this);
        return PopRegBuffer();
    };
    const auto origin = alloc_origin(node.GetOrigin());

    node.GetDestination()->Accept(*this);
    const auto destination = PopRegBuffer();

    const auto step = Registers().Allocate();
    bytecode.Write(Op::LoadConstant, {step, bytecode.AddConstant(1)});

    // loop 2..8 and loop 8..2 need a different step value
    const auto is_ascending = Registers().Allocate();
    bytecode.Write(Op::Cmp_LesserEq, {is_ascending, origin, destination});
    const i64 neg_jmp = bytecode.Write(Op::JumpWhenTrue, {is_ascending, SENTINEL});
    bytecode.Write(Op::Negate, {step, step});

    // we do a very short jump over the neg instruction if we're ascending
    PatchJumpForwardConditional(neg_jmp);
    Registers().Free(is_ascending);

    // counter's scope technically belongs to the loop, so we manually increment it here
    ++scope;
    const auto counter = Registers().Allocate();
    AddSymbol(node.GetCounterName(), counter);
    --scope;

    bytecode.Write(Op::Move, {counter, origin});
    Registers().Free(origin);

    return {destination, step, counter};
}

void BytecodeGenerator::HandlePendingSkips() {
    for (const auto [skip_target, has_condition] : CurrentLoop().pending_skips) {
        if (has_condition) {
            PatchJumpForwardConditional(skip_target);
        } else {
            PatchJumpForward(skip_target);
        }
    }
}

void BytecodeGenerator::HandlePendingBreaks() {
    for (const auto [loop_end, has_condition] : CurrentLoop().pending_breaks) {
        if (has_condition) {
            PatchJumpForwardConditional(loop_end);
        } else {
            PatchJumpForward(loop_end);
        }
    }
}

void BytecodeGenerator::HandleLoopControl(bool is_break, const NodePtr& condition) {
    const bool has_condition = condition != nullptr;
    i64 jump_index;

    if (has_condition) {
        // break/skip if cond
        condition->Accept(*this);
        const auto cond_reg = PopRegBuffer();

        jump_index = bytecode.Write(Op::JumpWhenTrue, {cond_reg, SENTINEL});
        Registers().Free(cond_reg);
    } else {
        // break/skip
        jump_index = bytecode.Write(Op::Jump, {SENTINEL});
    }

    auto& buffer = is_break ? CurrentLoop().pending_breaks : CurrentLoop().pending_skips;
    buffer.emplace_back(jump_index, has_condition);
}

void BytecodeGenerator::HandleInitializer(const Initializer& node, bool is_mutable) {
    const auto name = node.GetName();

    Register datum;

    if (const auto& init = node.GetInitializer()) {
        init->Accept(*this);

        // may be an identifier or constant
        const auto src = PopRegBuffer();

        datum = Registers().Allocate();
        bytecode.Write(Op::Move, {datum, src});
        Registers().Free(src);
    } else {
        datum = Registers().Allocate();
        bytecode.Write(Op::LoadConstant, {datum, bytecode.AddConstant(0.0)});
    }

    AddSymbol(name, datum);
}
} // namespace circe