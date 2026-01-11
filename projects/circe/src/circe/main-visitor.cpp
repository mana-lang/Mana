#include <algorithm>
#include <ranges>

#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

namespace circe {
using namespace mana::vm;
using namespace sigil::ast;

CirceVisitor::CirceVisitor()
    : total_registers {}
  , scope_depth {} {}

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

        const u64 jwf = slice.Write(jump_op, {lhs, 0xDEAD});
        FreeRegister(lhs);

        node.GetRight().Accept(*this);
        const u16 rhs = PopRegBuffer();
        slice.Write(Op::Move, {dst, rhs});

        slice.Patch(jwf, slice.Instructions().size() - (jwf + CJMP_OP_BYTES), 1);
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

void CirceVisitor::Visit(const Statement& node) {
    Log->error("Statement nodes should forward their visit to their child node(s).");
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
        datum = PopRegBuffer();
    } else {
        datum = AllocateRegister();
        slice.Write(Op::LoadConstant, {datum, slice.AddConstant(0.0)});
    }

    AddSymbol(name, datum);
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

void CirceVisitor::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const u16 cond_reg = PopRegBuffer();

    const u64 jmp_false = slice.Write(Op::JumpWhenFalse, {cond_reg, 0xDEAD});
    FreeRegister(cond_reg);

    node.GetThenBlock()->Accept(*this);

    if (const auto& else_branch = node.GetElseBranch()) {
        const u64 jmp_end = slice.Write(Op::Jump, {0xDEAD});

        slice.Patch(jmp_false, slice.Instructions().size() - (jmp_false + CJMP_OP_BYTES), 1);

        else_branch->Accept(*this);

        slice.Patch(jmp_end, slice.Instructions().size() - (jmp_end + JMP_OP_BYTES), 0);
    } else {
        slice.Patch(jmp_false, slice.Instructions().size() - (jmp_false + CJMP_OP_BYTES), 1);
    }
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

void CirceVisitor::AddSymbol(const std::string& name, u16 register_index) {
    symbols[name] = {register_index, scope_depth};
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
