#include <sigil/core/logger.hpp>

#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>

namespace sigil::ast {
using namespace mana::literals;

std::shared_ptr<Literal_F64> MakeLiteral(const Token& token) {
    return std::make_shared<Literal_F64>(token.AsF64());
}

/// module
auto Module::GetName() const -> std::string_view {
    return name;
}

auto Module::GetChildren() const -> const std::vector<Ptr>& {
    return children;
}

void Module::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// f64
Literal_F64::Literal_F64(const f64 value)
    : value(value) {}

f64 Literal_F64::Get() const {
    return value;
}

void Literal_F64::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// BinaryExpr
BinaryExpr::BinaryExpr(const ParseNode& node, const i64 depth) {
    const auto& tokens   = node.tokens;
    const auto& branches = node.branches;

    if (tokens.size() <= depth) {
        // we're in the leaf node
        left  = ConstructChild(*branches[0]);
        right = ConstructChild(*branches[1]);
        op    = tokens[0].text[0];
        return;
    }

    // we're in a parent node
    left  = std::shared_ptr<BinaryExpr>(new BinaryExpr(node, depth + 1));  // can't call make_shared cause private
    right = ConstructChild(*branches[branches.size() - depth]);
    op    = tokens[tokens.size() - depth].text[0];
}

BinaryExpr::BinaryExpr(const ParseNode& node)
    : BinaryExpr(node, 1) {}

BinaryExpr::BinaryExpr(const char op, const ParseNode& lhs, const ParseNode& rhs)
    : op(op)
    , left(ConstructChild(lhs))
    , right(ConstructChild(rhs)) {}

char BinaryExpr::GetOp() const {
    return op;
}

auto BinaryExpr::GetLeft() const -> const Node& {
    return *left;
}

auto BinaryExpr::GetRight() const -> const Node& {
    return *right;
}

void BinaryExpr::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

SIGIL_NODISCARD bool IsNumber(const TokenType token) {
    return token == TokenType::Lit_Float || token == TokenType::Lit_Int;
}

Node::Ptr BinaryExpr::ConstructChild(const ParseNode& node) {
    const auto& token = node.tokens[0];
    switch (node.rule) {
        using enum Rule;

    case Grouping:
        return ConstructChild(*node.branches[0]);

    case Term:
    case Factor:
        return std::make_shared<BinaryExpr>(token.text[0], *node.branches[0], *node.branches[1]);

    case Unary:
        if (token.type == TokenType::Op_Minus && IsNumber(node.branches[0]->tokens[0].type)) {
            return std::make_shared<UnaryExpr>(node);
        }
        break;
    case Literal:
        if (IsNumber(token.type)) {
            return MakeLiteral(token);
        }
        break;
    default:
        LogErr("Not a binary op");
        break;
    }

    LogErr("Erroneous input to BinaryOp");

    return nullptr;
}

/// UnaryExpr
UnaryExpr::UnaryExpr(const ParseNode& node)
    : op(node.tokens[0].text[0]) {
    const auto& next_node = *node.branches[0];
    if (next_node.rule == Rule::Literal) {
        val = MakeLiteral(next_node.tokens[0]);
        return;
    }

    val = std::make_shared<UnaryExpr>(node);
}

void UnaryExpr::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

char UnaryExpr::GetOp() const {
    return op;
}

auto UnaryExpr::GetVal() const -> const Node& {
    return *val;
}
}  // namespace sigil::ast