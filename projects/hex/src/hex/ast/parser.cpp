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
        goto next_node;
    }

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

next_node:
    if (not root.branches.empty()) {
        for (const auto& node : root.branches) {
            PrintAST(*node, prepend);
        }
    }

    if (not root.branches.empty() && root.IsRoot()) {
        Log("");
    }
}

bool Parser::IsPrimitive(TokenType token_type) const {
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

    const bool matched_something = Matched_Expression(node);

    return matched_something;
}

bool Parser::Matched_Expression(Node& node) {
    // const bool matched_something = Matched_Comparison(node) || Matched_Term(node) || Matched_Factor(node) ||
    //                                Matched_Unary(node) || Matched_Primary(node);

    return Matched_Equality(node); //
}

// literal = number | string | KW_true | KW_false | KW_null
bool Is_Literal(const TokenType token) {
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
bool Parser::Matched_Primary(Node& node) {
    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor_;
        return true;
    }

    if (CurrentToken().type == TokenType::Op_ParenLeft) {
        auto& grouping = node.NewBranch();
        grouping.rule  = Rule::Grouping;
        AddCycledTokenTo(grouping);

        if (Matched_Expression(grouping)) {
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

    if (not Is_Literal(CurrentToken().type)) {
        return false;
    }

    auto& primary {node.NewBranch()};
    primary.rule = Rule::Literal;
    AddCycledTokenTo(primary);
    return true;
}

// unary = ("-" | "!") expr
bool Parser::Matched_Unary(Node& node) {
    switch (CurrentToken().type) {
        using enum TokenType;

    case Op_Minus:
    case Op_LogicalNot: {
        auto& unary {node.NewBranch(Rule::Unary)};

        AddCycledTokenTo(unary);
        if (not Matched_Unary(unary)) {
            if (not Matched_Primary(unary)) {
                LogErr("Expected primary expression.");
                node.PopBranch();  // dangle should be okay since we immediately return
            } else
                break;

            return false;
        }
        break;
    }

    default:
        return Matched_Primary(node);
    }

    return true;
}

bool Is_FactorOp(const TokenType token) {
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
bool Parser::Matched_Factor(Node& node) {
    return Matched_BinaryExpr(node, Is_FactorOp, Matched_Unary, Rule::Factor);
}

bool Is_TermOp(const TokenType token) {
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
bool Parser::Matched_Term(Node& node) {
    return Matched_BinaryExpr(node, Is_TermOp, Matched_Factor, Rule::Term);
}

bool Is_ComparisonOp(const TokenType token) {
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

bool Parser::Matched_Comparison(Node& node) {
    return Matched_BinaryExpr(node, Is_ComparisonOp, Matched_Term, Rule::Comparison);
}

bool Is_EqualityOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_Equality:
    case Op_NotEqual:
        return true;
    default:
        return false;
    }
}

bool Parser::Matched_Equality(Node& node) {
    return Matched_BinaryExpr(node, Is_EqualityOp, Matched_Comparison, Rule::Equality);
}

bool Parser::Matched_BinaryExpr(
    Node&                node,
    const OpCheckerFnPtr is_valid_operator,
    const MatcherFnPtr   matched_operand,
    const Rule           rule
) {
    if (not (this->*matched_operand)(node)) {
        return false;
    }

    if (not is_valid_operator(CurrentToken().type)) {
        return true;
    }

    auto& binary_expr = node.NewBranch(rule);
    AddCycledTokenTo(binary_expr);

    binary_expr.AcquireBranchOf(node, node.branches.size() - 2);
    const auto expr_index = node.branches.size() - 1;

    if (not (this->*matched_operand)(node)) {
        LogErr("Expected expression");
        return false;
    }

    bool ret = true;
    while (is_valid_operator(CurrentToken().type)) {
        AddCycledTokenTo(binary_expr);

        if (not (this->*matched_operand)(node)) {
            LogErr("Incomplete expression");
            binary_expr.rule = Rule::Mistake;

            ret = false;
        }
    }
    binary_expr.AcquireBranchesOf(node, expr_index + 1);

    return ret;
}
}  // namespace hex