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

void MainVisitor::Visit(const Artifact& artifact) {
    for (const auto& child : artifact.GetChildren()) {
        child->Accept(*this);
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
        Log->error("Unknown Binary-Operator '{}'", node.GetOp());
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

void MainVisitor::Visit(const ArrayLiteral& node) {
    const auto& array_elems = node.GetValues();

    if (array_elems.empty()) {
        return;
    }

    const auto ConstructValues = [&array_elems, this] <typename T>() {
        std::vector<T> values;
        for (const auto& val : array_elems) {
            values.push_back(dynamic_cast<Literal<T>&>(*val).Get());
        }
        slice.Write(Op::Push, slice.AddConstants(values));
    };

    switch (node.GetType()) {
        using enum mana::PrimitiveType;

    case Float64:
        ConstructValues.operator()<f64>();
        return;
    case Uint64:
        ConstructValues.operator()<u64>();
        return;
    case Int64:
        ConstructValues.operator()<i64>();
        return;
    case Bool:
        ConstructValues.operator()<bool>();
        return;
    default:
        break;
    }

    Log->error("Unhandled array literal type '{}'", magic_enum::enum_name(node.GetType()));
}

void MainVisitor::Visit(const Statement& node) {
    node.Accept(*this);
    slice.Write(Op::Return);
}

void MainVisitor::Visit(const Scope& node) {
    node.Accept(*this);
}

void MainVisitor::Visit(const If& node) {
    constexpr u64 payload_bytes = 2;

    // 'if' only writes an early jwf
    node.GetCondition()->Accept(*this);
    const u64 jwf_index = slice.Write(Op::JumpWhenFalse, 0xDEAD);

    node.GetThenBlock()->Accept(*this);

    const auto compute_dist = [this](const u64 index) {
        u64 dist = slice.BackIndex() - index - payload_bytes;

        if (dist > std::numeric_limits<u16>::max()) {
            Log->error("Jump distance '{}' exceeded maximum jump size of {}",
                       dist,
                       std::numeric_limits<u16>::max()
            );
            dist = 0;
        }

        return dist;
    };

    const auto& else_branch = node.GetElseBranch();
    const u16   else_offset = else_branch == nullptr ? 0 : 3;
    slice.Patch(jwf_index, compute_dist(jwf_index) + else_offset);

    // else
    if (else_branch == nullptr) {
        return;
    }

    const u64 jmp_index = slice.Write(Op::Jump, 0xDEAD);

    else_branch->Accept(*this);
    slice.Patch(jmp_index, compute_dist(jmp_index));
}

void MainVisitor::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);

    if (node.GetOp().size() > 1) {
        Log->error("Unhandled unary expression");
        return;
    }

    switch (node.GetOp()[0]) {
    case '-':
        slice.Write(Op::Negate);
        break;
    case '!':
        slice.Write(Op::Not);
        break;
    default:
        Log->error("Invalid unary expression");
        break;
    }
}
} // namespace circe
