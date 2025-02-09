#include <sigil/ast/ast.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/core/logger.hpp>

namespace sigil::ast {
using namespace mana::literals;

auto Module::GetName() const -> std::string_view {
    return name;
}

auto Module::GetChildren() const -> const std::vector<Ptr>& {
    return children;
}

void Module::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

LiteralF64::LiteralF64(const f64 value)
    : value(value) {}

f64 LiteralF64::Get() const {
    return value;
}

void LiteralF64::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

BinaryOp::BinaryOp(const char op, const ParseNode& lhs, const ParseNode& rhs)
    : op(op) {
    ConstructChild(lhs, left);
    ConstructChild(rhs, right);
}

char BinaryOp::GetOp() const {
    return op;
}

auto BinaryOp::GetLeft() const -> const Node& {
    return *left;
}

auto BinaryOp::GetRight() const -> const Node& {
    return *right;
}

void BinaryOp::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

void BinaryOp::ConstructChild(const ParseNode& node, Ptr& target) {
    switch (node.rule) {
        using enum Rule;

    case Literal: {
        const auto& token = node.tokens[0];
        if (token.type == TokenType::Lit_Float) {
            target = std::make_shared<LiteralF64>(std::stof(token.text));
        }
        break;
    }
    default:
        LogErr("Not a binary op");
        break;
    }
}
}  // namespace sigil::ast