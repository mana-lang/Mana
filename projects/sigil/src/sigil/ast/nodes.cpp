#include "spdlog/fmt/bundled/chrono.h"
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

Node::Ptr MakeLiteral(const Token& token) {
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
    Log->error("Unexpected token for literal");
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

/// ArrayLiteral
ArrayLiteral::ArrayLiteral(const ParseNode& node) {
    // []
    if (node.branches.empty()) {
        return;
    }

    if (node.branches.size() > 1) {
        Log->error("ArrayLiteral may only contain one elem list");
    }

    for (const auto& elem_list = *node.branches[0];
         const auto& elem : elem_list.branches) {
        values.emplace_back(std::move(GetValue(*elem)));
    }
}

Node::Ptr ArrayLiteral::GetValue(const ParseNode& elem) {
    switch (elem.rule) {
        using enum Rule;

    case Grouping:
        // [()]
        if (elem.branches.empty()) {
            Log->warn("Empty grouping inside array literal");
            return nullptr;
        }

        // [(foo)]
        return GetValue(*elem.branches[0]);

    case ArrayLiteral:
        // [[1, 2, 3,], [4, 3, 2],]
        // in this case we still need to ensure the array is of "array-of-arrays" type
        // and adequately handle higher-dimensional arrays.
        // this is extremely important for linalg
        return std::make_shared<ast::ArrayLiteral>(elem);

    case Literal:
        // [12.4, 95.3]
        return MakeLiteral(elem.tokens[0]);

    case Equality:
    case Comparison:
    case Term:
    case Factor:
        // [8 + 5,]
        return std::make_shared<BinaryExpr>(elem);

    case Unary:
        // [-28, -99]
        return std::make_shared<UnaryExpr>(elem);

    default:
        Log->error("Unexpected rule in elem list");
        return nullptr;
    }
}

const std::vector<Node::Ptr>& ArrayLiteral::GetValues() const {
    return values;
}

const Node::Ptr& ArrayLiteral::operator[](const i64 index) const {
    return values.at(index);
}

void ArrayLiteral::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// BinaryExpr
BinaryExpr::BinaryExpr(const ParseNode& binary_node, const i64 depth) {
    const auto& tokens   = binary_node.tokens;
    const auto& branches = binary_node.branches;

    if (tokens.size() <= depth) {
        // we're in the leaf node
        left  = ConstructChild(*branches[0]);
        right = ConstructChild(*branches[1]);
        op    = tokens[0].text;
        return;
    }
    // we're in a parent node

    //                                  can't call make_shared cause private
    left  = std::shared_ptr<BinaryExpr>(new BinaryExpr(binary_node, depth + 1));
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
    using enum TokenType;
    return token == Lit_Float || token == Lit_Int;
}

SIGIL_NODISCARD bool IsBooleanLiteral(const TokenType token) {
    using enum TokenType;
    return token == Lit_true || token == Lit_false;
}

Node::Ptr BinaryExpr::ConstructChild(const ParseNode& operand_node) {
    const auto& token = operand_node.tokens[0];
    switch (operand_node.rule) {
        using enum Rule;

    case Grouping:
        return ConstructChild(*operand_node.branches[0]);

    case Comparison:
    case Term:
    case Factor:
        return std::make_shared<
            BinaryExpr>(token.text, *operand_node.branches[0], *operand_node.branches[1]);

    case Unary: {
        if (token.type == TokenType::Op_Minus) {
            if (IsNumber(operand_node.branches[0]->tokens[0].type)) {
                return std::make_shared<UnaryExpr>(operand_node);
            }
            // report error
            break;
        }
        if (token.type == TokenType::Op_LogicalNot) {
            if (IsBooleanLiteral(operand_node.branches[0]->tokens[0].type)) {
                return std::make_shared<UnaryExpr>(operand_node);
            }
            // report error
            break;
        }
    } break;
    case Literal:
        return MakeLiteral(token);

    default:
        Log->error("Not a binary op");
        break;
    }

    Log->error("Erroneous input to BinaryOp");

    return nullptr;
}

/// UnaryExpr
UnaryExpr::UnaryExpr(const ParseNode& unary_node)
    : op(unary_node.tokens[0].text) {
    const auto& operand_node = *unary_node.branches[0];

    if (operand_node.rule == Rule::Literal) {
        val = MakeLiteral(operand_node.tokens[0]);
        return;
    }

    val = std::make_shared<UnaryExpr>(operand_node);
}

void UnaryExpr::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

std::string_view UnaryExpr::GetOp() const {
    return op;
}

const Node& UnaryExpr::GetVal() const {
    return *val;
}
}  // namespace sigil::ast