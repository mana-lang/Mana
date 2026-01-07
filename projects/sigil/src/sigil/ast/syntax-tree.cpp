#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/source-file.hpp>
#include <sigil/ast/visitor.hpp>
#include <sigil/core/logger.hpp>

#include <mana/vm/primitive-type.hpp>

#include <magic_enum/magic_enum.hpp>

namespace sigil::ast {
using namespace mana::literals;

/// Literal Helpers
template <typename T>
auto MakeLiteral(const Token& token) {
    return std::make_shared<Literal<T>>(FetchTokenText(token));
}

template <>
auto MakeLiteral<bool>(const Token& token) {
    bool val = false;
    switch (token.type) {
    case TokenType::Lit_true:
        val = true;
    case TokenType::Lit_false:
        break;
    default:
        Log->critical("Bool conversion requested for non-bool token '{}'. "
                      "Defaulting to 'false'.",
                      magic_enum::enum_name(token.type));
    }

    return std::make_shared<Literal<bool>>(val);
}

auto MakeNoneLiteral() {
    return std::make_shared<Literal<void>>();
}

struct LiteralData {
    NodePtr             value;
    mana::PrimitiveType type;
};

LiteralData MakeLiteral(const Token& token) {
    switch (token.type) {
        using enum TokenType;

    case Lit_true:
    case Lit_false:
        return {MakeLiteral<bool>(token), mana::PrimitiveType::Bool};

    case Lit_Int:
        return {MakeLiteral<i64>(token), mana::PrimitiveType::Int64};

    case Lit_Float:
        return {MakeLiteral<f64>(token), mana::PrimitiveType::Float64};

    case Lit_none:
        return {MakeNoneLiteral(), mana::PrimitiveType::None};

    default:
        break;
    }

    Log->error("Unexpected token for literal");
    return {nullptr, mana::PrimitiveType::Invalid};
}

/// Artifact
auto Artifact::GetName() const -> std::string_view {
    return name;
}

auto Artifact::GetChildren() const -> const std::vector<NodePtr>& {
    return statements;
}

void Artifact::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

Scope::Scope(const ParseNode& node) {
    PropagateStatements(node, this);
}

void Scope::Accept(Visitor& visitor) const {
    for (const auto& stmt : statements) {
        stmt->Accept(visitor);
    }
}

const std::vector<NodePtr>& Scope::GetStatements() const {
    return statements;
}

If::If(const ParseNode& node) {
    switch (node.branches[0]->rule) {
    case Rule::Literal:
        condition = MakeLiteral(node.branches[0]->tokens[0]).value;
        condition_type = Rule::Literal;
        break;
    case Rule::Unary:
        condition = std::make_shared<UnaryExpr>(*node.branches[0]);
        condition_type = Rule::Unary;
        break;
    case Rule::Factor:
    case Rule::Comparison:
    case Rule::Equality:
    case Rule::Term:
    case Rule::Logical:
        condition = std::make_shared<BinaryExpr>(*node.branches[0]);
        condition_type = Rule::Expression;
        break;
    default:
        Log->error("Unexpected rule in if-block");
        condition_type = Rule::Undefined;
        return;
    }

    then_block = std::make_shared<Scope>(*node.branches[1]);

    if (node.branches.size() > 2) {
        auto& tail = *node.branches[2]->branches[0];
        if (tail.rule == Rule::Scope) {
            else_branch = std::make_shared<Scope>(tail);
        } else if (tail.rule == Rule::IfBlock) {
            else_branch = std::make_shared<If>(tail);
        } else {
            Log->error("Unexpected rule in else-branch");
        }
    } else {
        else_branch = nullptr;
    }
}

const NodePtr& If::GetCondition() const {
    return condition;
}

const NodePtr& If::GetThenBlock() const {
    return then_block;
}

const NodePtr& If::GetElseBranch() const {
    return else_branch;
}

Rule If::ConditionType() const {
    return condition_type;
}

void If::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Statement
Statement::Statement(const NodePtr&& node)
    : child(node) {}

void Statement::Accept(Visitor& visitor) const {
    child->Accept(visitor); // forward the visitor, statements don't do anything on their own
}

/// ArrayLiteral
ArrayLiteral::ArrayLiteral(const ParseNode& node)
    : type(mana::PrimitiveType::Invalid) {
    // []
    if (node.branches.empty()) {
        return;
    }

    if (node.branches.size() > 1) {
        Log->error("ArrayLiteral may only contain one elem list");
    }

    for (const auto& elem_list = *node.branches[0];
         const auto& elem : elem_list.branches) {
        values.emplace_back(std::move(ProcessValue(*elem)));
    }
}

NodePtr ArrayLiteral::ProcessValue(const ParseNode& elem) {
    switch (elem.rule) {
        using enum Rule;

    case Grouping:
        // [()]
        if (elem.branches.empty()) {
            Log->warn("Empty grouping inside array literal");
            return nullptr;
        }

        // [(foo)]
        return ProcessValue(*elem.branches[0]);

    case ArrayLiteral:
        // [[1, 2, 3,], [4, 3, 2],]
        // in this case we still need to ensure the array is of "array-of-arrays" type
        // and adequately handle higher-dimensional arrays.
        // this is extremely important for linalg
        return std::make_shared<ast::ArrayLiteral>(elem);

    case Literal:
        // [12.4, 95.3]
    {
        const auto literal = MakeLiteral(elem.tokens[0]);

        if (literal.type == mana::PrimitiveType::Invalid) {
            Log->error(
                "ArrayLiteral attempted to add invalid value '{}'",
                FetchTokenText(elem.tokens[0])
            );
            return nullptr;
        }

        // we want to deduce the array's type based on the first literal in the
        // elem_list so we start in Invalid, assign the type based on the first, and
        // any type changes past that raise an error
        if (literal.type != type) {
            if (type == mana::PrimitiveType::Invalid) {
                type = literal.type;
            } else {
                Log->warn(
                    fmt::runtime(
                        "ArrayLiteral is of type '{}', "
                        "but tried adding value of type '{}'"
                    ),
                    magic_enum::enum_name(literal.type)
                );
            }
        }
        return literal.value;
    }

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

const std::vector<NodePtr>& ArrayLiteral::GetValues() const {
    return values;
}

mana::PrimitiveType ArrayLiteral::GetType() const {
    return type;
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
        op    = FetchTokenText(tokens[0]);
        return;
    }
    // we're in a parent node

    //                                  can't call make_shared cause private
    left  = std::shared_ptr<BinaryExpr>(new BinaryExpr(binary_node, depth + 1));
    right = ConstructChild(*branches[branches.size() - depth]);
    op    = FetchTokenText(tokens[tokens.size() - depth]);
}

BinaryExpr::BinaryExpr(const ParseNode& node)
    : BinaryExpr(node, 1) {}

BinaryExpr::BinaryExpr(const std::string& op, const ParseNode& left, const ParseNode& right)
    : op(op)
    , left(ConstructChild(left))
    , right(ConstructChild(right)) {}

BinaryExpr::BinaryExpr(const std::string_view op, const ParseNode& left, const ParseNode& right)
    : op(op)
    , left(ConstructChild(left))
    , right(ConstructChild(right)) {}

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

NodePtr BinaryExpr::ConstructChild(const ParseNode& operand_node) {
    const auto& token = operand_node.tokens[0];
    switch (operand_node.rule) {
        using enum Rule;

    case Grouping:
        return ConstructChild(*operand_node.branches[0]);

    case Logical:
    case Equality:
    case Comparison:
    case Term:
    case Factor:
        return std::make_shared<BinaryExpr>(
            FetchTokenText(token), *operand_node.branches[0], *operand_node.branches[1]
        );

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
    }
    break;
    case Literal:
        return MakeLiteral(token).value;

    default:
        Log->error("Not a binary op");
        break;
    }

    Log->error("Erroneous input to BinaryOp");

    return nullptr;
}

/// UnaryExpr
UnaryExpr::UnaryExpr(const ParseNode& unary_node)
    : op(FetchTokenText(unary_node.tokens[0])) {
    const auto& operand_node = *unary_node.branches[0];

    if (operand_node.rule == Rule::Literal) {
        val = MakeLiteral(operand_node.tokens[0]).value;
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
} // namespace sigil::ast
