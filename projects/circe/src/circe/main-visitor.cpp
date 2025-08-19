#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <mana/vm/opcode.hpp>

namespace circe {
using namespace mana::vm;
using namespace sigil::ast;

Slice MainVisitor::GetSlice() const {
    return slice;
}

void MainVisitor::Visit(const Artifact& artifact) {
    for (const auto& child : artifact.GetChildren()) {
        child->Accept(*this);
        slice.Write(Op::Return);
    }
}

void MainVisitor::Visit(const BinaryExpr& node) {
    node.GetLeft().Accept(*this);
    node.GetRight().Accept(*this);

    const auto op = node.GetOp();

    switch (op[0]) {
    case '+':
        slice.Write(Op::Add);
        break;
    case '-':
        slice.Write(Op::Sub);
        break;
    case '*':
        slice.Write(Op::Mul);
        break;
    case '/':
        slice.Write(Op::Div);
        break;
    case '>':
        if (op.size() == 2 && op[1] == '=') {
            slice.Write(Op::Cmp_GreaterEq);
            break;
        }
        slice.Write(Op::Cmp_Greater);
        break;
    case '<':
        if (op.size() == 2 && op[1] == '=') {
            slice.Write(Op::Cmp_LesserEq);
            break;
        }
        slice.Write(Op::Cmp_Lesser);
        break;
    case '=':
        if (op.size() == 2 && op[1] == '=') {
            slice.Write(Op::Equals);
            break;
        }
    case '!':
        if (op.size() == 2 && op[1] == '=') {
            slice.Write(Op::NotEquals);
            break;
        }
    default:
        Log()->error("Unknown Binary-Operator '{}'", node.GetOp());
        break;
    }
}

void MainVisitor::Visit(const Literal<f64>& node) {
    slice.Write(Op::Push, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const Literal<i64>& node) {
    slice.Write(Op::Push, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const Literal<void>& node) {}

void MainVisitor::Visit(const Literal<bool>& node) {
    slice.Write(Op::Push, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);

    switch (node.GetOp()) {
    case '-':
        slice.Write(Op::Negate);
        break;
    case '!':
        slice.Write(Op::Not);
        break;
    default:
        Log()->error("Invalid unary expression");
        break;
    }
}
}  // namespace circe