#include <circe/core/logger.hpp>
#include <circe/bytecode-generator.hpp>

#include <sigil/ast/keywords.hpp>

#include <mana/vm/opcode.hpp>

#include <algorithm>
#include <ranges>

namespace circe {
using namespace mana::vm;
using namespace sigil::ast;

BytecodeGenerator::BytecodeGenerator()
    : total_registers {},
      scope_depth {} {}

ByteCode BytecodeGenerator::Bytecode() const {
    return bytecode;
}

void BytecodeGenerator::Visit(const Artifact& artifact) {
    for (const auto& statement : artifact.GetChildren()) {
        statement->Accept(*this);

        // prevent dangling register references
        ClearRegBuffer();
    }
    bytecode.Write(Op::Halt);
}

void BytecodeGenerator::Visit(const Scope& node) {
    EnterScope();

    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);
        ClearRegBuffer();
    }

    ExitScope();
}

void BytecodeGenerator::Visit(const MutableDataDeclaration& node) {
    HandleDeclaration(node, true);
}

void BytecodeGenerator::Visit(const DataDeclaration& node) {
    HandleDeclaration(node, false);
}

void BytecodeGenerator::Visit(const Identifier& node) {
    // even though input is assumed to be correct
    // we don't wanna fail silently if semantic analysis fails
    if (const auto it = symbols.find(node.GetName());
        it != symbols.end()) {
        reg_buffer.push_back(it->second.register_index);
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

    FreeRegister(rhs);
}

void BytecodeGenerator::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const auto cond_reg = PopRegBuffer();

    const i64 jmp_false = bytecode.Write(Op::JumpWhenFalse, {cond_reg, SENTINEL});
    FreeRegister(cond_reg);

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

    const i64 start_addr = bytecode.InstructionCount();

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

    const i64 start_addr = bytecode.InstructionCount();

    node.GetCondition()->Accept(*this);
    const auto condition = PopRegBuffer();

    // the jump out of the loop needs to happen immediately after condition evaluation
    const i64 jmp_end = bytecode.Write(Op::JumpWhenFalse, {condition, SENTINEL});
    FreeRegister(condition);

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

    const i64 start_addr = bytecode.InstructionCount();

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    // end of loop
    node.GetCondition()->Accept(*this);
    const auto condition = PopRegBuffer();
    JumpBackwardsConditional(Op::JumpWhenTrue, condition, start_addr);

    FreeRegister(condition);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopRange& node) {
    EnterLoop();

    const auto range = PerformRangeLoopSetup(node);

    // we add or subtract 1 since ranges are inclusive
    // this lets us compare to "equals" rather than jwf greater/lesser
    bytecode.Write(Op::Add, {range.end, range.end, range.step});

    const auto cond = AllocateRegister();

    // loop starts here
    const i64 start_addr = bytecode.InstructionCount();

    bytecode.Write(Op::Equals, {cond, range.counter, range.end});
    const i64 exit = bytecode.Write(Op::JumpWhenTrue, {cond, SENTINEL});

    node.GetBody()->Accept(*this);
    HandlePendingSkips();

    bytecode.Write(Op::Add, {range.counter, range.counter, range.step});
    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    FreeRegister(cond);
    FreeRegister(range.counter);
    FreeRegister(range.step);
    FreeRegister(range.end);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopRangeMutable& node) {
    EnterLoop();

    const auto range = PerformRangeLoopSetup(node);

    // allocate persistent registers before loop
    const auto zero = AllocateRegister();
    bytecode.Write(Op::LoadConstant, {zero, bytecode.AddConstant(0)});

    const auto cond = AllocateRegister();

    // loop starts here
    const i64 start_addr = bytecode.InstructionCount();

    // for mutable loop counters, we can't just compare to equals
    // so we do something slightly more involved
    // (end - counter) * step >= 0
    const auto diff = AllocateRegister();
    bytecode.Write(Op::Sub, {diff, range.end, range.counter});
    bytecode.Write(Op::Mul, {diff, diff, range.step});

    bytecode.Write(Op::Cmp_GreaterEq, {cond, diff, zero});
    const i64 exit = bytecode.Write(Op::JumpWhenFalse, {cond, SENTINEL});

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    bytecode.Write(Op::Add, {range.counter, range.counter, range.step});

    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    FreeRegister(range.end);
    FreeRegister(range.step);
    FreeRegister(range.counter);
    FreeRegister(cond);
    FreeRegister(diff);
    FreeRegister(zero);

    HandlePendingBreaks();

    ExitLoop();
}

void BytecodeGenerator::Visit(const LoopFixed& node) {
    EnterLoop();

    const auto counter = AllocateRegister();
    bytecode.Write(Op::LoadConstant, {counter, bytecode.AddConstant(0)});

    const auto step = AllocateRegister();
    bytecode.Write(Op::LoadConstant, {step, bytecode.AddConstant(1)});

    // we haven't entered the body yet, but the target belongs to that scope
    // since the target may be a literal, we need to do it like this
    ++scope_depth;
    node.GetCountTarget()->Accept(*this);
    const auto target = PopRegBuffer();
    --scope_depth;

    const i64 start_addr = bytecode.InstructionCount();

    // while the parser tries to guard against negative counts, they may be undetectable at compile time
    // in that case, the loop would end immediately
    const auto cond = AllocateRegister();
    bytecode.Write(Op::Cmp_Lesser, {cond, counter, target});
    const i64 exit = bytecode.Write(Op::JumpWhenFalse, {cond, SENTINEL});

    node.GetBody()->Accept(*this);

    HandlePendingSkips();

    // increment and bounce back to start
    bytecode.Write(Op::Add, {counter, counter, step});

    JumpBackwards(start_addr);
    PatchJumpForwardConditional(exit);

    HandlePendingBreaks();

    FreeRegister(cond);
    FreeRegister(target);
    FreeRegister(step);
    FreeRegister(counter);

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
    auto dst = AllocateRegister();

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
        FreeRegister(src);
        return;
    }

    bytecode.Write(op, {dst, src});
    reg_buffer.push_back(dst);
    FreeRegister(src);
}

void BytecodeGenerator::Visit(const BinaryExpr& node) {
    const auto op_text = node.GetOp();

    // logical ops need special treatment as they are control flow due to short-circuiting
    const auto jump = [this, &node](const Op jump_op) {
        node.GetLeft().Accept(*this);

        const auto lhs = PopRegBuffer();
        const auto dst = AllocateRegister();
        bytecode.Write(Op::Move, {dst, lhs});

        const i64 jwf = bytecode.Write(jump_op, {lhs, SENTINEL});
        FreeRegister(lhs);

        node.GetRight().Accept(*this);
        const auto rhs = PopRegBuffer();
        bytecode.Write(Op::Move, {dst, rhs});

        PatchJumpForwardConditional(jwf);

        reg_buffer.push_back(dst);
        FreeRegister(rhs);
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
    auto dst = AllocateRegister();

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
    reg_buffer.push_back(dst);
    FreeRegisters({lhs, rhs});
}

void BytecodeGenerator::Visit(const ArrayLiteral& array) {
    const auto& array_elems = array.GetValues();
    if (array_elems.empty()) {
        return;
    }

    // for now we just index arrays as separate values
    // eventually we'll need a way to tell the VM to construct an array from all these sequential constants
    for (const auto& val : array_elems) {
        val->Accept(*this);
    }

    // TODO: add 'MakeArray' opcode to VM
}

void BytecodeGenerator::Visit(const Literal<f64>& literal) {
    CreateLiteral(literal);
}

void BytecodeGenerator::Visit(const Literal<i64>& literal) {
    CreateLiteral(literal);
}

void BytecodeGenerator::Visit(const Literal<void>& node) {}

void BytecodeGenerator::Visit(const Literal<bool>& literal) {
    CreateLiteral(literal);
}

bool BytecodeGenerator::IsConditionalJumpOp(const Op op) const {
    return op == Op::JumpWhenTrue || op == Op::JumpWhenFalse;
}

void BytecodeGenerator::JumpBackwards(i64 target_index) {
    bytecode.Write(Op::Jump, {CalcJump(target_index, false, false)});
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
                                  ? bytecode.InstructionCount() - (target_index + jump_bytes)
                                  : target_index - (bytecode.InstructionCount() + jump_bytes);

    if (not JumpIsWithinBounds(jump_distance)) {
        Log->error("Internal Compiler Error: Jump distance out of bounds");
        return SENTINEL;
    }

    return static_cast<Register>(jump_distance);
}

Register BytecodeGenerator::AllocateRegister() {
    if (free_regs.empty()) {
        return total_registers++;
    }

    const auto slot = free_regs.back();
    free_regs.pop_back();

    return slot;
}

void BytecodeGenerator::FreeRegister(Register reg) {
    for (const Register r : free_regs) {
        if (r == reg) {
            return;
        }
    }

    if (not RegisterIsOwned(reg)) {
        free_regs.push_back(reg);
    }
}

void BytecodeGenerator::FreeRegisters(const std::initializer_list<Register> regs) {
    for (const auto reg : regs) {
        FreeRegister(reg);
    }
}

void BytecodeGenerator::FreeRegisters(const std::vector<Register>& regs) {
    for (const auto reg : regs) {
        FreeRegister(reg);
    }
}

bool BytecodeGenerator::RegisterIsOwned(Register reg) {
    for (const auto& val : std::views::values(symbols)) { // NOLINT(*-use-anyofallof)
        if (val.register_index == reg) {
            return true;
        }
    }

    return false;
}

Register BytecodeGenerator::PopRegBuffer() {
    if (reg_buffer.empty()) {
        Log->error("Internal Compiler Error: Register stack underflow");
        return 0;
    }

    const auto slot = reg_buffer.back();
    reg_buffer.pop_back();

    return slot;
}

void BytecodeGenerator::ClearRegBuffer() {
    FreeRegisters(reg_buffer);
    reg_buffer.clear();
}

void BytecodeGenerator::EnterScope() {
    ++scope_depth;
}

void BytecodeGenerator::ExitScope() {
    std::vector<std::string_view> to_remove;
    to_remove.reserve(symbols.size());

    for (const auto& [name, symbol] : symbols) {
        if (symbol.scope_depth == scope_depth) {
            to_remove.push_back(name);
        }
    }

    for (const auto& name : to_remove) {
        RemoveSymbol(name);
    }
    --scope_depth;
}

void BytecodeGenerator::EnterLoop() {
    loop_stack.emplace_back();
}

void BytecodeGenerator::ExitLoop() {
    loop_stack.pop_back();
}

void BytecodeGenerator::AddSymbol(const std::string_view name, Register index) {
    symbols[name] = {index, scope_depth};
}

void BytecodeGenerator::RemoveSymbol(const std::string_view name) {
    if (not symbols.contains(name)) {
        Log->warn("Internal Compiler Error: Attempted to remove non-existent symbol '{}'", name);
        return;
    }

    // prevent duplicate entries in e.g. 'data x = y'
    auto reg = symbols[name].register_index;
    symbols.erase(name);
    FreeRegister(reg);
}

BytecodeGenerator::LoopContext& BytecodeGenerator::CurrentLoop() {
    return loop_stack.back();
}

BytecodeGenerator::RangeLoopRegisters BytecodeGenerator::PerformRangeLoopSetup(const LoopRange& node) {
    const auto alloc_origin = [this](const NodePtr& origin) {
        if (origin == nullptr) {
            const auto reg = AllocateRegister();
            bytecode.Write(Op::LoadConstant, {reg, bytecode.AddConstant(0)});
            return reg;
        }

        origin->Accept(*this);
        return PopRegBuffer();
    };
    const auto origin = alloc_origin(node.GetOrigin());

    node.GetDestination()->Accept(*this);
    const auto destination = PopRegBuffer();

    const auto step = AllocateRegister();
    bytecode.Write(Op::LoadConstant, {step, bytecode.AddConstant(1)});

    // loop 2..8 and loop 8..2 need a different step value
    const auto is_ascending = AllocateRegister();
    bytecode.Write(Op::Cmp_LesserEq, {is_ascending, origin, destination});
    const i64 neg_jmp = bytecode.Write(Op::JumpWhenTrue, {is_ascending, SENTINEL});
    bytecode.Write(Op::Negate, {step, step});

    // we do a very short jump over the neg instruction if we're ascending
    PatchJumpForwardConditional(neg_jmp);
    FreeRegister(is_ascending);

    // counter's scope technically belongs to the loop, so we manually increment it here
    ++scope_depth;
    const auto counter = AllocateRegister();
    AddSymbol(node.GetCounterName(), counter);
    --scope_depth;

    bytecode.Write(Op::Move, {counter, origin});
    FreeRegister(origin);

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
        FreeRegister(cond_reg);
    } else {
        // break/skip
        jump_index = bytecode.Write(Op::Jump, {SENTINEL});
    }

    auto& buffer = is_break ? CurrentLoop().pending_breaks : CurrentLoop().pending_skips;
    buffer.emplace_back(jump_index, has_condition);
}

void BytecodeGenerator::HandleDeclaration(const Initializer& node, bool is_mutable) {
    const auto name = node.GetName();

    Register datum;

    if (const auto& init = node.GetInitializer()) {
        init->Accept(*this);

        // may be an identifier or constant
        const auto src = PopRegBuffer();
        datum          = AllocateRegister();
        bytecode.Write(Op::Move, {datum, src});

        FreeRegister(src);
    } else {
        datum = AllocateRegister();
        bytecode.Write(Op::LoadConstant, {datum, bytecode.AddConstant(0.0)});
    }

    AddSymbol(name, datum);
}
} // namespace circe
