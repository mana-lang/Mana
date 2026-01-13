#include <algorithm>
#include <ranges>

#include <circe/core/logger.hpp>
#include <circe/circe-visitor.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

namespace circe {
using namespace mana::vm;
using namespace sigil::ast;

CirceVisitor::CirceVisitor()
    : total_registers {},
      scope_depth {} {}

Slice CirceVisitor::GetSlice() const {
    return slice;
}

void CirceVisitor::Visit(const Artifact& artifact) {
    for (const auto& statement : artifact.GetChildren()) {
        statement->Accept(*this);

        // prevent dangling register references
        ClearRegBuffer();
    }
    slice.Write(Op::Halt);
}

void CirceVisitor::Visit(const Scope& node) {
    ++scope_depth;
    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);
        ClearRegBuffer();
    }

    CleanupCurrentScope();
    --scope_depth;
}

void CirceVisitor::Visit(const DataDeclaration& node) {
    const std::string name(node.GetName());
    if (symbols.contains(name)) {
        Log->error("Redefinition of '{}'", name);
        return;
    }

    u16 datum;

    if (const auto& init = node.GetInitializer()) {
        init->Accept(*this);

        // may be an identifier or constant
        u16 src = PopRegBuffer();
        datum   = AllocateRegister();
        slice.Write(Op::Move, {datum, src});

        FreeRegister(src);
    } else {
        datum = AllocateRegister();
        slice.Write(Op::LoadConstant, {datum, slice.AddConstant(0.0)});
    }

    AddSymbol(name, datum, node.IsMutable());
}

void CirceVisitor::Visit(const Identifier& node) {
    const std::string name(node.GetName());
    if (const auto it = symbols.find(name);
        it != symbols.end()) {
        reg_buffer.push_back(it->second.register_index);
        return;
    }

    Log->warn("Undefined identifier '{}'", name);
    slice.Write(Op::Halt);
}

void CirceVisitor::Visit(const Assignment& node) {
    const std::string name(node.GetIdentifier());
    const auto it = symbols.find(name);
    if (it == symbols.end()) {
        Log->warn("Undefined identifier '{}'", name);
        return;
    }

    if (not it->second.is_mutable) {
        Log->error("Cannot assign to immutable data '{}'", name);
        slice.Write(Op::Err);
        return;
    }

    node.GetValue()->Accept(*this);
    u16 rhs             = PopRegBuffer();
    std::string_view op = node.GetOp();
    if (op.size() == 1) {
        slice.Write(Op::Move, {it->second.register_index, rhs});
    } else {
        u16 lhs = it->second.register_index;

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

        slice.Write(operation, {lhs, lhs, rhs});
    }

    FreeRegister(rhs);
}

void CirceVisitor::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const u16 cond_reg = PopRegBuffer();

    const i64 jmp_false = slice.Write(Op::JumpWhenFalse, {cond_reg, 0xDEAD});
    FreeRegister(cond_reg);

    node.GetThenBlock()->Accept(*this);

    if (const auto& else_branch = node.GetElseBranch()) {
        const i64 jmp_end = slice.Write(Op::Jump, {0xDEAD});

        slice.Patch(jmp_false, CalcJumpDistance(jmp_false, true), 1);

        else_branch->Accept(*this);

        slice.Patch(jmp_end, CalcJumpDistance(jmp_end), 0);
    } else {
        slice.Patch(jmp_false, CalcJumpDistance(jmp_false, true), 1);
    }
}

void CirceVisitor::Visit(const Loop& node) {
    const i64 start_addr = slice.InstructionCount();
    skip_buffer.push_back(start_addr);

    node.GetBody()->Accept(*this);

    // end of loop
    slice.Write(Op::Jump, {CalcJumpBackwards(start_addr)});

    for (const auto [break_jump, is_conditional] : break_buffer) {
        slice.Patch(break_jump, CalcJumpDistance(break_jump, is_conditional), is_conditional ? 1 : 0);
    }

    skip_buffer.pop_back();
    break_buffer.pop_back();
}

void CirceVisitor::Visit(const LoopIf& node) {}
void CirceVisitor::Visit(const LoopIfPost& node) {}
void CirceVisitor::Visit(const LoopRange& node) {}
void CirceVisitor::Visit(const LoopFixed& node) {}

void CirceVisitor::Visit(const Break& node) {
    // break-if, has to be handled by loop
    if (not node.HasCondition()) {
        break_buffer.emplace_back(slice.Write(Op::Jump, {0xDEAD}), false);
        return;
    }

    node.GetCondition()->Accept(*this);
    const u16 condition = PopRegBuffer();

    break_buffer.emplace_back(slice.Write(Op::JumpWhenTrue, {condition, 0xDEAD}), true);
    FreeRegister(condition);
}

void CirceVisitor::Visit(const Skip& node) {
    const i64 start_addr = skip_buffer.back();

    // skip if
    if (node.HasCondition()) {
        node.GetCondition()->Accept(*this);
        const u16 condition = PopRegBuffer();

        slice.Write(Op::JumpWhenTrue, {condition, CalcJumpBackwards(start_addr, true)});
        FreeRegister(condition);

        return;
    }

    // skip
    slice.Write(Op::Jump, {CalcJumpBackwards(start_addr)});
}

void CirceVisitor::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);

    if (node.GetOp().size() > 1) {
        Log->error("Unhandled unary expression");
        return;
    }

    u16 src = PopRegBuffer();
    u16 dst = AllocateRegister();

    Op op;
    switch (node.GetOp()[0]) {
    case '-':
        op = Op::Negate;
        break;
    case '!':
        op = Op::Not;
        break;
    default:
        Log->error("Invalid unary expression");
        FreeRegister(src);
        FreeRegister(PopRegBuffer());
        return;
    }

    slice.Write(op, {dst, src});
    reg_buffer.push_back(dst);
    FreeRegister(src);
}

void CirceVisitor::Visit(const BinaryExpr& node) {
    const auto op_str = node.GetOp();

    // logical ops need special treatment as they are control flow due to short-circuiting
    const auto jump = [this, &node](const Op jump_op) {
        node.GetLeft().Accept(*this);

        const u16 lhs = PopRegBuffer();
        const u16 dst = AllocateRegister();
        slice.Write(Op::Move, {dst, lhs});

        const i64 jwf = slice.Write(jump_op, {lhs, 0xDEAD});
        FreeRegister(lhs);

        node.GetRight().Accept(*this);
        const u16 rhs = PopRegBuffer();
        slice.Write(Op::Move, {dst, rhs});

        slice.Patch(jwf, CalcJumpDistance(jwf, true), 1);
        reg_buffer.push_back(dst);
        FreeRegister(rhs);
    };


    if (op_str == "&&") {
        jump(Op::JumpWhenFalse);
        return;
    }

    if (op_str == "||") {
        jump(Op::JumpWhenTrue);
        return;
    }

    node.GetLeft().Accept(*this);
    node.GetRight().Accept(*this);

    u16 rhs = PopRegBuffer();
    u16 lhs = PopRegBuffer();
    u16 dst = AllocateRegister();

    Op op;
    switch (op_str[0]) {
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
    case '>':
        if (op_str.size() == 2 && op_str[1] == '=') {
            op = Op::Cmp_GreaterEq;
            break;
        }
        op = Op::Cmp_Greater;
        break;
    case '<':
        if (op_str.size() == 2 && op_str[1] == '=') {
            op = Op::Cmp_LesserEq;
            break;
        }
        op = Op::Cmp_Lesser;
        break;
    case '=':
        if (op_str.size() == 2 && op_str[1] == '=') {
            op = Op::Equals;
            break;
        }
        FreeRegisters({lhs, rhs});
        return; // prevent accidental fallthrough
    case '!':
        if (op_str.size() == 2 && op_str[1] == '=') {
            op = Op::NotEquals;
            break;
        }
        FreeRegisters({lhs, rhs});
        return;
    default:
        Log->error("Unknown Binary Operator '{}'", node.GetOp());
        FreeRegisters({lhs, rhs});
        return;
    }

    slice.Write(op, {dst, lhs, rhs});
    reg_buffer.push_back(dst);
    FreeRegisters({lhs, rhs});
}

void CirceVisitor::Visit(const ArrayLiteral& array) {
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

void CirceVisitor::Visit(const Literal<f64>& literal) {
    CreateLiteral(literal);
}

void CirceVisitor::Visit(const Literal<i64>& literal) {
    CreateLiteral(literal);
}

void CirceVisitor::Visit(const Literal<void>& node) {}

void CirceVisitor::Visit(const Literal<bool>& literal) {
    CreateLiteral(literal);
}

bool JumpIsWithinBounds(const i64 distance) {
    return distance <= std::numeric_limits<i16>::max()
           && distance >= std::numeric_limits<i16>::min();
}

u16 CirceVisitor::CalcJumpDistance(const i64 jump_index, const bool is_conditional) const {
    const auto jump_bytes = is_conditional ? CJMP_OP_BYTES : JMP_OP_BYTES;

    const i64 jump_distance = slice.InstructionCount() - (jump_index + jump_bytes);

    if (not JumpIsWithinBounds(jump_distance)) {
        Log->error("Jump distance out of bounds");
        return 0xDEAD;
    }

    return static_cast<u16>(jump_distance);
}

// i hate how similar these are, but adding another bool to calcjumpdist would probably be bad
u16 CirceVisitor::CalcJumpBackwards(const i64 jump_index, const bool is_conditional) const {
    const auto jump_bytes = is_conditional ? CJMP_OP_BYTES : JMP_OP_BYTES;

    const i64 jump_distance = jump_index - (slice.InstructionCount() + jump_bytes);

    if (not JumpIsWithinBounds(jump_distance)) {
        Log->error("Jump distance out of bounds");
        return 0xDEAD;
    }

    return static_cast<u16>(jump_distance);
}

u16 CirceVisitor::AllocateRegister() {
    if (free_regs.empty()) {
        return total_registers++;
    }

    const u16 slot = free_regs.back();
    free_regs.pop_back();

    return slot;
}

void CirceVisitor::FreeRegister(u16 reg) {
    if (not RegisterIsOwned(reg)) {
        free_regs.push_back(reg);
    }
}

void CirceVisitor::FreeRegisters(std::initializer_list<u16> regs) {
    for (const auto reg : regs) {
        FreeRegister(reg);
    }
}

void CirceVisitor::FreeRegisters(const std::vector<u16>& regs) {
    for (const auto reg : regs) {
        FreeRegister(reg);
    }
}

bool CirceVisitor::RegisterIsOwned(u16 reg) {
    for (const auto& val : std::views::values(symbols)) { // NOLINT(*-use-anyofallof)
        if (val.register_index == reg) {
            return true;
        }
    }
    return false;
}

u16 CirceVisitor::PopRegBuffer() {
    if (reg_buffer.empty()) {
        Log->error("Internal Compiler Error: Register stack underflow");
        return 0;
    }

    const u16 slot = reg_buffer.back();
    reg_buffer.pop_back();

    return slot;
}

void CirceVisitor::ClearRegBuffer() {
    FreeRegisters(reg_buffer);
    reg_buffer.clear();
}

void CirceVisitor::CleanupCurrentScope() {
    std::vector<std::string> to_remove;
    for (const auto& [name, symbol] : symbols) {
        if (symbol.scope_depth == scope_depth) {
            // delegate removal to avoid iterator invalidation
            to_remove.push_back(name);
        }
    }

    for (const auto& name : to_remove) {
        RemoveSymbol(name);
    }
}

void CirceVisitor::AddSymbol(const std::string& name, u16 register_index, bool is_mutable) {
    symbols[name] = {register_index, scope_depth, is_mutable};
}

void CirceVisitor::RemoveSymbol(const std::string& name) {
    if (not symbols.contains(name)) {
        Log->warn("Attempted to remove non-existent symbol '{}'", name);
        return;
    }

    // prevent duplicate entries in e.g. 'data x = y'
    u16 reg = symbols[name].register_index;
    symbols.erase(name);
    FreeRegister(reg);
}
} // namespace circe
