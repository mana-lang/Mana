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
    for (const auto& child : artifact.GetChildren()) {
        child->Accept(*this);
    }
}

void MainVisitor::Visit(const BinaryExpr& node) {
    const auto op = node.GetOp();

    // logical ops need special treatment as they are control flow due to short-circuiting
    const auto jump = [this, &node] (const Op jump_op) {
        node.GetLeft().Accept(*this);

        const u64 idx = slice.Write(jump_op, 0xDEAD);
        slice.Write(Op::Pop);

        node.GetRight().Accept(*this);
        slice.Patch(idx, ComputeJumpDist(idx));
    };

    if (op.size() == 2) {
        if (op == "&&") {
            jump(Op::JumpWhenFalse);
            return;;
        }

        if (op == "||") {
            jump(Op::JumpWhenTrue);
            return;
        }
    }


    node.GetLeft().Accept(*this);
    node.GetRight().Accept(*this);

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

void MainVisitor::Visit(const Literal<f64>& literal) {
    slice.Write(Op::Push, slice.AddConstant(literal.Get()));
}

void MainVisitor::Visit(const Literal<i64>& literal) {
    slice.Write(Op::Push, slice.AddConstant(literal.Get()));
}

void MainVisitor::Visit(const Literal<void>& node) {}

void MainVisitor::Visit(const Literal<bool>& literal) {
    slice.Write(Op::Push, slice.AddConstant(literal.Get()));
}

void MainVisitor::Visit(const ArrayLiteral& array) {
    const auto& array_elems = array.GetValues();

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

    switch (array.GetType()) {
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

    Log->error("Unhandled array literal type '{}'", magic_enum::enum_name(array.GetType()));
}

void MainVisitor::Visit(const Statement& node) {
    node.Accept(*this);
    slice.Write(Op::Return);
}

void MainVisitor::Visit(const Scope& node) {
    node.Accept(*this);
}

void MainVisitor::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    const u64 jmp_false = slice.Write(Op::JumpWhenFalse, 0xDEAD);

    // 'true' path
    // pop condition before executing then-block
    slice.Write(Op::Pop);
    node.GetThenBlock()->Accept(*this);

    // need to jump over 'false' path to reach end of block
    const u64 jmp_end = slice.Write(Op::Jump, 0xDEAD);

    // 'false' path
    // jwf lands here, skipping regular jmp
    slice.Patch(jmp_false, ComputeJumpDist(jmp_false));

    // if we don't pop here, the if-condition never gets cleaned up
    slice.Write(Op::Pop);

    if (const auto& else_branch = node.GetElseBranch()) {
        else_branch->Accept(*this);
    }

    slice.Patch(jmp_end, ComputeJumpDist(jmp_end));
}

void MainVisitor::Visit(const Datum& node) {
    if (node.GetInitializer() != nullptr) {
        node.GetInitializer()->Accept(*this);
    }
    // TODO: Store the value in a symbol table or local variable slot
    // For now, just evaluate the initializer and leave the value on the stack
}

void MainVisitor::Visit(const Identifier& node) {
    Log->warn("Identifier reference '{}'", node.GetName());
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
}//
} // namespace circe
