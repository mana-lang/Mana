#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

namespace circe {
using namespace mana::vm;
using namespace sigil::ast;

Slice MainVisitor::GetSlice() const {
    return slice;
}

u64 MainVisitor::ComputeJumpDist(const u64 index) const {
    constexpr u64 payload_bytes = 2;

    u64 dist = slice.BackIndex() - index - payload_bytes;
    if (dist > std::numeric_limits<u16>::max()) {
        Log->error("Jump distance '{}' exceeded maximum jump size of {}",
                   dist,
                   std::numeric_limits<u16>::max()
        );
        dist = 0;
    }

    return dist;
}

void MainVisitor::Visit(const Artifact& artifact) {
    for (const auto& statement : artifact.GetChildren()) {
        statement->Accept(*this);

        // prevent dangling register references
        reg_stack.clear();
    }
    slice.Write(Op::Halt);
}

constexpr u8 CJMP_OP_BYTES = 5;
constexpr u8 JMP_OP_BYTES = 3;

void MainVisitor::Visit(const BinaryExpr& node) {
    const auto op_str = node.GetOp();

    // logical ops need special treatment as they are control flow due to short-circuiting
    const auto jump = [this, &node](const Op jump_op) {
        node.GetLeft().Accept(*this);

        const u16 lhs = PopRegStack();
        const u16 dst = AllocateRegister();
        slice.Write(Op::Move, {dst, lhs});

        const u64 jwf = slice.Write(jump_op, {lhs, 0xDEAD});

        node.GetRight().Accept(*this);
        const u16 rhs = PopRegStack();
        slice.Write(Op::Move, {dst, rhs});

        slice.Patch(jwf, slice.Instructions().size() - (jwf + CJMP_OP_BYTES), 1);
        reg_stack.push_back(dst);
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

    u16 rhs = PopRegStack();
    u16 lhs = PopRegStack();
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
        return; // prevent accidental fallthrough
    case '!':
        if (op_str.size() == 2 && op_str[1] == '=') {
            op = Op::NotEquals;
            break;
        }
        return;
    default:
        Log->error("Unknown Binary Operator '{}'", node.GetOp());
        return;
    }

    slice.Write(op, {dst, lhs, rhs});
    reg_stack.push_back(dst);
}

void MainVisitor::Visit(const Literal<f64>& literal) {
    CreateLiteral(literal);
}

void MainVisitor::Visit(const Literal<i64>& literal) {
    CreateLiteral(literal);
}

void MainVisitor::Visit(const Literal<bool>& literal) {
    CreateLiteral(literal);
}

void MainVisitor::Visit(const Literal<void>& node) {}

void MainVisitor::Visit(const ArrayLiteral& array) {
    const auto& array_elems = array.GetValues();
    if (array_elems.empty()) return;

    // for now we just index arrays as separate values
    // eventually we'll need a way to tell the VM to construct an array from all these sequential constants
    for (const auto& val : array_elems) {
        val->Accept(*this);
    }

    // TODO: add 'MakeArray' opcode to VM
}

void MainVisitor::Visit(const Statement& node) {
    node.Accept(*this);
    //Log->error("Statement nodes should probably not be in the AST");
}

void MainVisitor::Visit(const Scope& node) {
    node.Accept(*this);
}

void MainVisitor::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const u16 cond_reg = PopRegStack();

    const u64 jmp_false = slice.Write(Op::JumpWhenFalse, {cond_reg, 0xDEAD});

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

void MainVisitor::Visit(const DataDeclaration& node) {
    const std::string name(node.GetName());
    if (symbols.contains(name)) {
        Log->error("Redefinition of '{}'", name);
        return;
    }

    u16 datum;

    if (const auto& init = node.GetInitializer()) {
        init->Accept(*this);
        datum = PopRegStack();
    } else {
        datum = AllocateRegister();
        slice.Write(Op::LoadConstant, {datum, slice.AddConstant(0.0)});
    }

    symbols[name] = datum;
}

void MainVisitor::Visit(const Identifier& node) {
    const std::string name(node.GetName());
    if (const auto it = symbols.find(name); it != symbols.end()) {
        reg_stack.push_back(it->second);
        return;
    }

    Log->warn("Undefined identifier '{}'", name);
    reg_stack.push_back(0);
}

u16 MainVisitor::AllocateRegister() {
    return next_slot++;
}

u16 MainVisitor::PopRegStack() {
    if (reg_stack.empty()) {
        Log->error("Internal Compiler Error: Register stack underflow");
        return 0;
    }

    const u16 slot = reg_stack.back();
    reg_stack.pop_back();

    return slot;
}

void MainVisitor::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);

    if (node.GetOp().size() > 1) {
        Log->error("Unhandled unary expression");
        return;
    }

    u16 src = PopRegStack();
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
        PopRegStack();
        return;
    }

    slice.Write(op, {dst, src});
    reg_stack.push_back(dst);
}
} // namespace circe
