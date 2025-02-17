#include <sigil/core/logger.hpp>

#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/visitor.hpp>

namespace sigil::ast {
using namespace mana::literals;

template <typename T>
auto MakeLiteral(const Token& token) {
    return std::make_shared<Literal<T>>(token.As<T>());
}

auto MakeNullLiteral() {
    return std::make_shared<Literal<void>>();
}

auto MakeLiteral(const Token& token) -> std::shared_ptr<Node> {
    switch (token.type) {
        using enum TokenType;

    case Lit_true:
    case Lit_false:
        return MakeLiteral<bool>(token);

    case Lit_Int:
        return MakeLiteral<i64>(token);

    case Lit_Float:
        return MakeLiteral<f64>(token);

    case Lit_null:
        return MakeNullLiteral();

    default:
        break;
    }
    return nullptr;
}

/// Artifact
auto Artifact::GetName() const -> std::string_view {
    return name;
}

auto Artifact::GetChildren() const -> const std::vector<Ptr>& {
    return children;
}

void Artifact::Accept(Visitor& visitor) const {
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
        op    = tokens[0].text;
        return;
    }

    // we're in a parent node
    left  = std::shared_ptr<BinaryExpr>(new BinaryExpr(node, depth + 1));  // can't call make_shared cause private
    right = ConstructChild(*branches[branches.size() - depth]);
    op    = tokens[tokens.size() - depth].text;
}

BinaryExpr::BinaryExpr(const ParseNode& node)
    : BinaryExpr(node, 1) {}

BinaryExpr::BinaryExpr(const std::string& op, const ParseNode& lhs, const ParseNode& rhs)
    : op(op)
    , left(ConstructChild(lhs))
    , right(ConstructChild(rhs)) {}

std::string_view BinaryExpr::GetOp() const {
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

    case Comparison:
    case Term:
    case Factor:
        return std::make_shared<BinaryExpr>(token.text, *node.branches[0], *node.branches[1]);

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
        Log->error("Not a binary op");
        break;
    }

    Log->error("Erroneous input to BinaryOp");

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