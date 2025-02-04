#include <hex/ast/parser.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>

namespace hex {
using namespace ast;

Parser::Parser(const TokenStream&& tokens)
    : tokens_ {tokens}
    , cursor_ {}
    , ast_ {Rule::Undefined} {}

Parser::Parser(const TokenStream& tokens)
    : tokens_ {tokens}
    , cursor_ {}
    , ast_ {Rule::Undefined} {}

bool Parser::Parse() {
    const auto& top_token = tokens_.front();

    if (top_token.type != TokenType::_module_) {
        LogErr(
            "Improper token stream format. Top-level token was: '{}' instead of "
            "'_module_'",
            magic_enum::enum_name(top_token.type)
        );
        return false;
    }

    ast_.rule = Rule::Module;
    ast_.tokens.push_back(tokens_.front());

    cursor_ = 1;
    while (ProgressedAST(ast_)) {}

    if (CurrentToken().type != TokenType::Eof) {
        // LogErr("It appears we did not parse the entire file.");
        return true;  // TODO: handle this error and return false instead
    }
    return true;
}

auto Parser::ViewAST() const -> const Node& {
    return ast_;
}

auto Parser::ViewTokens() const -> const TokenStream& {
    return tokens_;
}

void Parser::PrintAST() const {
    Log("Printing AST for module '{}'", ast_.tokens[0].text);
    PrintAST(ast_);
}

void Parser::PrintAST(const Node& root, std::string prepend) const {
    if (root.rule == Rule::Module) {
        Log("[Module] -> {}", root.tokens[0].text);

    } else {
        Log("{}[{}]", prepend, magic_enum::enum_name(root.rule));

        prepend.append("== ");

        if (not root.tokens.empty()) {
            std::ranges::replace(prepend, '=', '-');

            for (const auto& [type, text, position] : root.tokens) {
                if (type == TokenType::Terminator) {
                    continue;
                }
                Log("{} [{}] -> {}", prepend, magic_enum::enum_name(type), text);
            }

            std::ranges::replace(prepend, '-', '=');
        }
    }

    if (not root.branches.empty()) {
        for (const auto& node : root.branches) {
            PrintAST(*node, prepend);
        }
    }

    if (not root.branches.empty() && root.IsRoot()) {
        Log("");
    }
}

bool IsPrimitive(const TokenType token_type) {
    switch (token_type) {
        using enum TokenType;

    case KW_i32:
    case KW_i64:

    case KW_u32:
    case KW_u64:

    case KW_f32:
    case KW_f64:

    case KW_char:
    case KW_string:
    case KW_byte:
        return true;

    default:
        return false;
    }
}

auto Parser::CurrentToken() const -> const Token& {
    return tokens_[cursor_];
}

auto Parser::PeekNextToken() const -> const Token& {
    return tokens_[cursor_ + 1];
}

auto Parser::NextToken() -> const Token& {
    return tokens_[++cursor_];
}

auto Parser::GetAndCycleToken() -> const Token& {
    return tokens_[cursor_++];
}

// inclusive
void Parser::AddTokensTo(Node& node, const TokenType delimiter) {
    do {
        node.tokens.push_back(GetAndCycleToken());
    } while (CurrentToken().type != delimiter);
}

void Parser::AddTokensTo(Node& node, const i64 count) {
    for (i64 i = 0; i < count; ++i) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::AddCurrentTokenTo(Node& node) const {
    if (cursor_ < tokens_.size()) {
        node.tokens.push_back(CurrentToken());
    }
}

void Parser::AddCycledTokenTo(Node& node) {
    if (cursor_ < tokens_.size()) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::TransmitTokens(TokenStream& sender, TokenStream& receiver) const {
    for (const auto& send : sender) {
        receiver.push_back(send);
    }

    sender.clear();
}

void Parser::TransmitTokens(Node& sender, Node& receiver, TokenRange range) const {
    const auto [breadth, offset] = range;

    if (breadth + offset > sender.tokens.size()) {
        LogErr("TransmitTokens error: range was too large");
        return;
    }

    for (i64 i = 0; i < breadth; ++i) {
        receiver.tokens.emplace_back(sender.tokens[offset]);
    }
}

bool Parser::ProgressedAST(Node& node) {
    // don't process eof (final token) before quitting
    if (cursor_ + 1 >= tokens_.size() - 1) {
        return false;
    }

    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor_;
    }

    const bool Matchedsomething = MatchedExpression(node);

    return Matchedsomething;
}

bool Parser::MatchedExpression(Node& node) {
    return MatchedEquality(node);
}

// literal = number | string | KW_true | KW_false | KW_null
bool IsLiteral(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Lit_Int:
    case Lit_Float:

    case Lit_Char:
    case Lit_String:

    case Lit_true:
    case Lit_false:
    case Lit_null:
        return true;
    default:
        return false;
    }
}

// primary  = literal | grouping
// grouping = "(" expr ")"
bool Parser::MatchedPrimary(Node& node) {
    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor_;
        return true;
    }

    if (CurrentToken().type == TokenType::Op_ParenLeft) {
        auto& grouping = node.NewBranch();
        grouping.rule  = Rule::Grouping;
        AddCycledTokenTo(grouping);

        if (MatchedExpression(grouping)) {
            if (grouping.branches.size() > 1) {
                LogErr("Grouping may not contain more than one expression");
                grouping.rule = Rule::Mistake;
                return false;
            }
            if (CurrentToken().type == TokenType::Op_ParenRight) {
                AddCycledTokenTo(grouping);

                if (grouping.branches.size() < 1) {
                    LogErr("Grouping must contain at least one expression");
                    return false;
                }

                // guaranteed to be valid expression by now
                return true;
            }
        }
    }

    if (not IsLiteral(CurrentToken().type)) {
        return false;
    }

    auto& primary {node.NewBranch()};
    primary.rule = Rule::Literal;
    AddCycledTokenTo(primary);
    return true;
}

// unary = ("-" | "!") expr
bool Parser::MatchedUnary(Node& node) {
    switch (CurrentToken().type) {
        using enum TokenType;

    case Op_Minus:
    case Op_LogicalNot: {
        auto& unary {node.NewBranch(Rule::Unary)};

        AddCycledTokenTo(unary);
        if (not MatchedUnary(unary)) {
            if (not MatchedPrimary(unary)) {
                LogErr("Expected primary expression.");
                node.PopBranch();  // dangle should be okay since we immediately return
            } else
                break;

            return false;
        }
        break;
    }

    default:
        return MatchedPrimary(node);
    }

    return true;
}

bool IsFactorOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_FwdSlash:
    case Op_Asterisk:
        return true;
    default:
        return false;
    }
}

// factor = unary ( ('/' | '*') unary )*
bool Parser::MatchedFactor(Node& node) {
    return MatchedBinaryExpr(node, IsFactorOp, MatchedUnary, Rule::Factor);
}

bool IsTermOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_Minus:
    case Op_Plus:
        return true;
    default:
        return false;
    }
}

// term = factor ( ('-' | '+') factor)*
bool Parser::MatchedTerm(Node& node) {
    return MatchedBinaryExpr(node, IsTermOp, MatchedFactor, Rule::Term);
}

bool IsComparisonOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_GreaterThan:
    case Op_GreaterEqual:
    case Op_LessThan:
    case Op_LessEqual:
        return true;
    default:
        return false;
    }
}

bool Parser::MatchedComparison(Node& node) {
    return MatchedBinaryExpr(node, IsComparisonOp, MatchedTerm, Rule::Comparison);
}

bool IsEqualityOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_Equality:
    case Op_NotEqual:
        return true;
    default:
        return false;
    }
}

bool Parser::MatchedEquality(Node& node) {
    return MatchedBinaryExpr(node, IsEqualityOp, MatchedComparison, Rule::Equality);
}

bool Parser::MatchedBinaryExpr(
    Node&                node,
    const OpCheckerFnPtr Isvalid_operator,
    const MatcherFnPtr   Matchedoperand,
    const Rule           rule
) {
    if (not(this->*Matchedoperand)(node)) {
        return false;
    }

    if (not Isvalid_operator(CurrentToken().type)) {
        return true;
    }

    auto& binary_expr = node.NewBranch(rule);
    AddCycledTokenTo(binary_expr);

    binary_expr.AcquireBranchOf(node, node.branches.size() - 2);
    const auto expr_index = node.branches.size() - 1;

    if (not(this->*Matchedoperand)(node)) {
        LogErr("Expected expression");
        return false;
    }

    bool ret = true;
    while (Isvalid_operator(CurrentToken().type)) {
        AddCycledTokenTo(binary_expr);

        if (not(this->*Matchedoperand)(node)) {
            LogErr("Incomplete expression");
            binary_expr.rule = Rule::Mistake;

            ret = false;
        }
    }
    binary_expr.AcquireBranchesOf(node, expr_index + 1);

    return ret;
}
}  // namespace hex