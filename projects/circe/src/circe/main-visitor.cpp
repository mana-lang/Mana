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

    switch (node.GetOp()) {
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

    default:
        Log->error("Unknown Binary Operator");
        break;
    }
}

void MainVisitor::Visit(const Literal<f64>& node) {
    slice.Write(Op::Push_Float, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const Literal<i64>& node) {
    /// TODO: change this to int, obviously
    slice.Write(Op::Push_Float, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const Literal<void>& node) {}

void MainVisitor::Visit(const Literal<bool>& node) {
    // slice.Write(Op::Push_Bool, slice.AddConstant(node.Get()));
}

void MainVisitor::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);

    if (node.GetOp() == '-') {
        slice.Write(Op::Negate);
    }
}
}  // namespace circe