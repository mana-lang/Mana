#include <ranges>
#include <sigil/ast/keywords.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/source-file.hpp>
#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>
#include <sigil/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <sigil/ast/keywords.hpp>

#include <spdlog/fmt/bundled/chrono.h>

namespace sigil::ast {
using namespace mana::literals;


/// Literal Helpers
template <typename T>
auto MakeLiteral(const Token& token) {
    return std::make_shared<Literal<T>>(FetchTokenText(token));
}

template <>
auto MakeLiteral<std::string>(const Token& token) {
    return std::make_shared<StringLiteral>(FetchTokenText(token));
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
                      magic_enum::enum_name(token.type)
        );
    }

    return std::make_shared<Literal<bool>>(val);
}

struct LiteralData {
    NodePtr value;
    std::string_view type;
};

LiteralData MakeLiteral(const Token& token) {
    switch (token.type) {
        using enum TokenType;

    case Lit_true:
    case Lit_false:
        return {MakeLiteral<bool>(token), PrimitiveName(PrimitiveType::Bool)};

    case Lit_Int:
        return {MakeLiteral<i64>(token), PrimitiveName(PrimitiveType::I64)};

    case Lit_Float:
        return {MakeLiteral<f64>(token), PrimitiveName(PrimitiveType::F64)};

    case Lit_String:
        return {MakeLiteral<std::string>(token), PrimitiveName(PrimitiveType::String)};

    case Lit_none:
        Log->error("Internal Compiler Error: Attempted to manifest 'none' literal");
        return {nullptr, PrimitiveName(PrimitiveType::None)};

    default:
        break;
    }

    Log->error("Unexpected token for literal");
    return {nullptr, PrimitiveName(PrimitiveType::None)};
}

/// Artifact
Artifact::Artifact(const std::string_view name, const ParseNode& node)
    : name(name) {
    for (auto& decl : node.branches) {
        declarations.emplace_back(CreateDeclaration(*decl));
    }
}

auto Artifact::GetName() const -> std::string_view {
    return name;
}

auto Artifact::GetChildren() const -> const std::vector<NodePtr>& {
    return declarations;
}

void Artifact::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// FunctionDeclaration
FunctionDeclaration::FunctionDeclaration(const ParseNode& node) {
    name = FetchTokenText(node.tokens[0]);

    std::string_view param_type;
    for (const auto& param : std::views::reverse(node.branches[0]->branches)) {
        if (param->tokens.size() == 2) {
            param_type = FetchTokenText(param->tokens[1]);
        }

        parameters.emplace_back(FetchTokenText(param->tokens[0]), param_type);
    }

    // fix the order after handling type propagation
    std::ranges::reverse(parameters);

    if (node.tokens.size() == 2) {
        return_type = FetchTokenText(node.tokens[1]);
    } else {
        return_type = PrimitiveName(PrimitiveType::None);
    }

    body = std::make_shared<Scope>(*node.branches[1]);
}

std::string_view FunctionDeclaration::GetName() const {
    return name;
}

std::span<const Parameter> FunctionDeclaration::GetParameters() const {
    return parameters;
}

const NodePtr& FunctionDeclaration::GetBody() const {
    return body;
}

std::string_view FunctionDeclaration::GetReturnType() const {
    return return_type;
}

void FunctionDeclaration::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

Invocation::Invocation(const ParseNode& node) {
    identifier = FetchTokenText(node.tokens[0]);

    for (const auto& arg : node.branches) {
        arguments.emplace_back(CreateExpression(*arg));
    }
}

std::string_view Invocation::GetIdentifier() const {
    return identifier;
}

const std::vector<NodePtr>& Invocation::GetArguments() const {
    return arguments;
}

void Invocation::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Binding
Initializer::Initializer(const ParseNode& node)
    : initializer {nullptr} {
    const auto& tokens = node.tokens;

    // data keyword is irrelevant to AST
    for (const auto& token : tokens) {
        if (token.type == TokenType::Identifier) {
            name = FetchTokenText(token);
            break;
        }
    }

    for (int i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::Op_Colon) {
            // AST input is assumed to be correct, so we don't need to bounds check
            type = FetchTokenText(tokens[i + 1]);
            break;
        }
    }

    if (not node.branches.empty()) {
        initializer = CreateExpression(*node.branches[0]);
    }
}

std::string_view Initializer::GetName() const {
    return name;
}

std::string_view Initializer::GetTypeName() const {
    return type;
}

const NodePtr& Initializer::GetInitializer() const {
    return initializer;
}

bool Initializer::HasTypeAnnotation() const {
    return not type.empty();
}

void Initializer::Accept(Visitor& visitor) const {
    Log->warn("Binding should never be visited directly");
}

/// DataDeclaration
DataDeclaration::DataDeclaration(const ParseNode& node)
    : Initializer {node} {}

void DataDeclaration::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// MutableDataDeclaration
MutableDataDeclaration::MutableDataDeclaration(const ParseNode& node)
    : Initializer {node} {}

void MutableDataDeclaration::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Statement
Statement::Statement(NodePtr&& node)
    : child(std::move(node)) {}

const NodePtr& Statement::GetChild() const {
    return child;
}

void Statement::Accept(Visitor& visitor) const {
    child->Accept(visitor); // forward the visitor, statements don't do anything on their own
}


/// If
If::If(const ParseNode& node) {
    condition = CreateExpression(*node.branches[0]);

    then_block = std::make_shared<Scope>(*node.branches[1]);

    if (node.branches.size() > 2) {
        auto& tail = *node.branches[2]->branches[0];
        if (tail.rule == Rule::Scope) {
            else_branch = std::make_shared<Scope>(tail);
        } else if (tail.rule == Rule::If) {
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

void If::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Loop
Loop::Loop(const ParseNode& node) {
    body = std::make_shared<Scope>(*node.branches[0]);
}

const NodePtr& Loop::GetBody() const {
    return body;
}

void Loop::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopIf
LoopIf::LoopIf(const ParseNode& node)
    : condition {CreateExpression(*node.branches[0])},
      body {std::make_shared<Scope>(*node.branches[1])} {}

const NodePtr& LoopIf::GetCondition() const {
    return condition;
}

const NodePtr& LoopIf::GetBody() const {
    return body;
}

void LoopIf::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopIfPost
LoopIfPost::LoopIfPost(const ParseNode& node)
    : condition {CreateExpression(*node.branches[1])},
      body {std::make_shared<Scope>(*node.branches[0])} {}

const NodePtr& LoopIfPost::GetCondition() const {
    return condition;
}

const NodePtr& LoopIfPost::GetBody() const {
    return body;
}

void LoopIfPost::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopRange
LoopRange::LoopRange(const ParseNode& node) {
    counter = FetchTokenText(node.tokens[0]);

    if (node.branches.size() == 2) {
        origin      = nullptr;
        destination = CreateExpression(*node.branches[0]);
        body        = std::make_shared<Scope>(*node.branches[1]);
        return;
    }

    // it's either a partial or full range
    origin      = CreateExpression(*node.branches[0]);
    destination = CreateExpression(*node.branches[1]);
    body        = std::make_shared<Scope>(*node.branches[2]);
}

const NodePtr& LoopRange::GetOrigin() const {
    return origin;
}

const NodePtr& LoopRange::GetDestination() const {
    return destination;
}

const NodePtr& LoopRange::GetBody() const {
    return body;
}

std::string_view LoopRange::GetCounterName() const {
    return counter;
}

void LoopRange::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopRangeMutable
LoopRangeMutable::LoopRangeMutable(const ParseNode& node)
    : LoopRange {node} {}

void LoopRangeMutable::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopFixed
LoopFixed::LoopFixed(const ParseNode& node) {
    count = CreateExpression(*node.branches[0]);
    body  = std::make_shared<Scope>(*node.branches[1]);
}

const NodePtr& LoopFixed::GetCountTarget() const {
    return count;
}

const NodePtr& LoopFixed::GetBody() const {
    return body;
}

void LoopFixed::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// LoopControl
LoopControl::LoopControl(const ParseNode& node)
    : condition {nullptr} {
    if (node.tokens.size() > 2 && node.tokens[2].type == TokenType::Identifier) {
        label = FetchTokenText(node.tokens[2]);
    }

    if (node.branches.empty()) {
        return;
    }

    // LoopControl can only have up to 1 branch
    condition = CreateExpression(*node.branches[0]);
}

const NodePtr& LoopControl::GetCondition() const {
    return condition;
}

std::string_view LoopControl::GetLabel() const {
    return label;
}

bool LoopControl::HasLabel() const {
    return not label.empty();
}

void LoopControl::Accept(Visitor& visitor) const {
    Log->warn("LoopControl should never be visited directly");
}

/// Break
Break::Break(const ParseNode& node)
    : LoopControl {node} {}

void Break::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Skip
Skip::Skip(const ParseNode& node)
    : LoopControl {node} {}

void Skip::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Return
Return::Return(const ParseNode& node) {
    // TODO: verify this
    if (node.branches.empty()
        || (node.branches[0]->rule == Rule::Identifier && FetchTokenText(node.branches[0]->tokens[0]) == std::string(
                PrimitiveName(
                    PrimitiveType::None
                )
            ))) {
        // it's either 'return' or 'return none' which are identical
        return;
    }
    expr = CreateExpression(*node.branches[0]);
}

const NodePtr& Return::GetExpression() const {
    return expr;
}

void Return::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Assignment
Assignment::Assignment(const ParseNode& node) {
    identifier = FetchTokenText(node.tokens[0]);
    op         = FetchTokenText(node.tokens[1]);
    value      = CreateExpression(*node.branches[0]);
}

std::string_view Assignment::GetIdentifier() const {
    return identifier;
}

const NodePtr& Assignment::GetValue() const {
    return value;
}

std::string_view Assignment::GetOp() const {
    return op;
}

void Assignment::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// Scope
Scope::Scope(const ParseNode& node) {
    for (const auto& stmt : node.branches) {
        using enum Rule;

        switch (stmt->rule) {
        case Return:
            AddStatement<class Return>(*stmt);
            break;
        case Invocation:
            AddStatement<class Invocation>(*stmt);
            break;
        case If:
            AddStatement<class If>(*stmt);
            break;
        case Loop:
            AddStatement<class Loop>(*stmt);
            break;
        case LoopIf:
            AddStatement<class LoopIf>(*stmt);
            break;
        case LoopIfPost:
            AddStatement<class LoopIfPost>(*stmt);
            break;
        case LoopRange:
            if (stmt->tokens[0].type == TokenType::KW_mut) {
                // mut token is useless past this point
                stmt->tokens[0] = stmt->tokens[1];
                stmt->tokens.pop_back();
                AddStatement<LoopRangeMutable>(*stmt);
                break;
            }
            AddStatement<class LoopRange>(*stmt);
            break;
        case LoopFixed:
            AddStatement<class LoopFixed>(*stmt);
            break;
        case LoopControl:
            if (stmt->tokens[0].type == TokenType::KW_break) {
                AddStatement<Break>(*stmt);
                break;
            }
            if (stmt->tokens[0].type == TokenType::KW_skip) {
                AddStatement<Skip>(*stmt);
                break;
            }
            Log->error("Unexpected loop control statement. Token was '{}'",
                       magic_enum::enum_name(stmt->tokens[0].type)
            );
            break;
        default:
            if (auto decl = CreateDeclaration(*stmt)) {
                AddStatement(std::move(decl));
            } else if (auto expr = CreateExpression(*stmt)) {
                AddStatement(std::move(expr));
            } else {
                Log->error("Expected statement");
            }
            break;
        }
    }
}


void Scope::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

const std::vector<NodePtr>& Scope::GetStatements() const {
    return statements;
}

/// Identifier
Identifier::Identifier(const ParseNode& node)
    : name(FetchTokenText(node.tokens[0])) {}

std::string_view Identifier::GetName() const {
    return name;
}

void Identifier::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

BinaryExpr::BinaryExpr(const ParseNode& node)
    : BinaryExpr(node, 1) {}

BinaryExpr::BinaryExpr(const std::string_view op, const ParseNode& left, const ParseNode& right)
    : op(op),
      left(CreateExpression(left)),
      right(CreateExpression(right)) {}

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

/// BinaryExpr
BinaryExpr::BinaryExpr(const ParseNode& binary_node, const i64 depth) {
    const auto& tokens   = binary_node.tokens;
    const auto& branches = binary_node.branches;

    if (tokens.size() <= depth) {
        // we're in the leaf node
        left  = CreateExpression(*branches[0]);
        right = CreateExpression(*branches[1]);
        op    = FetchTokenText(tokens[0]);
        return;
    }
    // we're in a parent node

    //                                  can't call make_shared cause private
    left  = std::shared_ptr<BinaryExpr>(new BinaryExpr(binary_node, depth + 1));
    right = CreateExpression(*branches[branches.size() - depth]);
    op    = FetchTokenText(tokens[tokens.size() - depth]);
}

/// StringLiteral
StringLiteral::StringLiteral(const std::string_view sv) {
    // unescape newlines
    string.reserve(sv.size());
    i64 last_append {};
    for (i64 i = 0; i < sv.size() - 1; ++i) {
        if (sv[i] == '\\' && sv[i + 1] == 'n') {
            string.append(sv.substr(last_append, i - last_append));
            string.push_back('\n');
            last_append = i++ + 2;
        }
    }
    string.append(sv.substr(last_append, sv.size() - last_append));
}

std::string_view StringLiteral::Get() const {
    return string;
}

void StringLiteral::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

/// ArrayLiteral
ListLiteral::ListLiteral(const ParseNode& node) {
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

const std::vector<NodePtr>& ListLiteral::GetValues() const {
    return values;
}

std::string_view ListLiteral::GetType() const {
    return type;
}

void ListLiteral::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

NodePtr ListLiteral::ProcessValue(const ParseNode& elem) {
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

    case ListLiteral:
        // [[1, 2, 3,], [4, 3, 2],]
        // in this case we still need to ensure the array is of "array-of-arrays" type
        // and adequately handle higher-dimensional arrays.
        // this is extremely important for linalg
        return std::make_shared<class ListLiteral>(elem);

    case Literal:
        // [12.4, 95.3]
        return MakeLiteral(elem.tokens[0]).value;

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

SIGIL_NODISCARD bool IsNumber(const TokenType token) {
    using enum TokenType;
    return token == Lit_Float || token == Lit_Int;
}

SIGIL_NODISCARD bool IsBooleanLiteral(const TokenType token) {
    using enum TokenType;
    return token == Lit_true || token == Lit_false;
}

/// UnaryExpr
UnaryExpr::UnaryExpr(const ParseNode& node)
    : op(FetchTokenText(node.tokens[0])),
      val(CreateExpression(*node.branches[0])) {}

void UnaryExpr::Accept(Visitor& visitor) const {
    visitor.Visit(*this);
}

std::string_view UnaryExpr::GetOp() const {
    return op;
}

const Node& UnaryExpr::GetVal() const {
    return *val;
}

NodePtr CreateExpression(const ParseNode& node) {
    using enum Rule;

    const auto& token = node.tokens.empty() ? Token {} : node.tokens[0];

    switch (node.rule) {
    case Invocation:
        return std::make_shared<class Invocation>(node);
    case Assignment:
        return std::make_shared<class Assignment>(node);
    case Grouping:
        return CreateExpression(*node.branches[0]);
    case Literal:
        return MakeLiteral(token).value;
    case Identifier:
        return std::make_shared<class Identifier>(node);
    case ListLiteral:
        return std::make_shared<class ListLiteral>(node);
    case Unary:
        return std::make_shared<UnaryExpr>(node);
    case Factor:
    case Term:
    case Comparison:
    case Equality:
    case Logical:
        return std::make_shared<BinaryExpr>(node);
    default:
        Log->trace("Failed expression check for '{}'", magic_enum::enum_name(node.rule));
        return nullptr;
    }
}

NodePtr CreateDeclaration(const ParseNode& node) {
    using enum Rule;

    switch (node.rule) {
    case FunctionDeclaration:
        return std::make_shared<class FunctionDeclaration>(node);
    case DataDeclaration:
        return std::make_shared<class DataDeclaration>(node);
    case MutableDataDeclaration:
        return std::make_shared<class MutableDataDeclaration>(node);
    default:
        Log->trace("Failed declaration check for '{}'", magic_enum::enum_name(node.rule));
        return nullptr;
    }
}
} // namespace sigil::ast
