#include <sigil/ast/parser.hpp>
#include <sigil/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <fstream>
#include <sigil/ast/nodes.hpp>

namespace sigil {
using namespace ast;
using namespace mana::literals;

Parser::Parser(const TokenStream&& tokens)
    : tokens {tokens}
    , cursor {}
    , parse_tree {Rule::Undefined} {}

Parser::Parser(const TokenStream& tokens)
    : tokens {tokens}
    , cursor {}
    , parse_tree {Rule::Undefined} {}

bool Parser::Parse() {
    const auto& top_token = tokens.front();

    if (top_token.type != TokenType::_artifact_) {
        LogErr(
            "Improper token stream format. Top-level token was: '{}' instead of {}",
            magic_enum::enum_name(top_token.type),
            magic_enum::enum_name(TokenType::_artifact_)
        );
        return false;
    }

    parse_tree.rule = Rule::Artifact;
    parse_tree.tokens.push_back(tokens.front());

    cursor = 1;
    while (ProgressedParseTree(parse_tree)) {}

    ConstructAST(parse_tree);

    if (CurrentToken().type != TokenType::Eof) {
        // LogErr("It appears we did not parse the entire file.");
        return true;  // TODO: handle this error and return false instead
    }

    return true;
}

auto Parser::ViewParseTree() const -> const ParseNode& {
    return parse_tree;
}

auto Parser::ViewTokens() const -> const TokenStream& {
    return tokens;
}

auto Parser::ViewAST() const -> Node* {
    return syntax_tree.get();
}

void Parser::PrintParseTree() const {
    Log("Printing AST for artifact '{}'\n\n{}", parse_tree.tokens[0].text, EmitParseTree(parse_tree));
}

void Parser::EmitParseTree(const std::string_view file_name) const {
    std::ofstream out {std::string(file_name)};

    out << EmitParseTree(parse_tree);

    out.close();
}

std::string Parser::EmitParseTree(const ParseNode& root, std::string prepend) const {
    std::string ret = "";
    if (root.rule == Rule::Artifact) {
        ret = fmt::format("[Module] -> {}\n\n", root.tokens[0].text);

    } else {
        ret.append(fmt::format("{}[{}]\n", prepend, magic_enum::enum_name(root.rule)));

        prepend.append("== ");

        if (not root.tokens.empty()) {
            std::ranges::replace(prepend, '=', '-');

            for (const auto& [type, text, position] : root.tokens) {
                if (type == TokenType::Terminator) {
                    continue;
                }
                ret.append(fmt::format("{} [{}] -> {}\n", prepend, magic_enum::enum_name(type), text));
            }

            std::ranges::replace(prepend, '-', '=');
        }
    }

    if (not root.branches.empty()) {
        for (auto& node : root.branches) {
            ret.append(EmitParseTree(*node, prepend));
        }
    }

    if (not root.branches.empty() && root.IsRoot()) {
        ret.append("\n");
    }

    return ret;
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
    return tokens[cursor];
}

auto Parser::PeekNextToken() const -> const Token& {
    return tokens[cursor + 1];
}

auto Parser::NextToken() -> const Token& {
    return tokens[++cursor];
}

auto Parser::GetAndCycleToken() -> const Token& {
    return tokens[cursor++];
}

// inclusive
void Parser::AddTokensTo(ParseNode& node, const TokenType delimiter) {
    do {
        node.tokens.push_back(GetAndCycleToken());
    } while (CurrentToken().type != delimiter);
}

void Parser::AddTokensTo(ParseNode& node, const i64 count) {
    for (i64 i = 0; i < count; ++i) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::AddCurrentTokenTo(ParseNode& node) const {
    if (cursor < tokens.size()) {
        node.tokens.push_back(CurrentToken());
    }
}

void Parser::AddCycledTokenTo(ParseNode& node) {
    if (cursor < tokens.size()) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::TransmitTokens(TokenStream& sender, TokenStream& receiver) const {
    for (const auto& send : sender) {
        receiver.push_back(send);
    }

    sender.clear();
}

void Parser::TransmitTokens(ParseNode& sender, ParseNode& receiver, TokenRange range) const {
    const auto [breadth, offset] = range;

    if (breadth + offset > sender.tokens.size()) {
        LogErr("TransmitTokens error: range was too large");
        return;
    }

    for (i64 i = 0; i < breadth; ++i) {
        receiver.tokens.emplace_back(sender.tokens[offset]);
    }
}

bool Parser::ProgressedParseTree(ParseNode& node) {
    // don't process eof (final token) before quitting
    if (cursor + 1 >= tokens.size() - 1) {
        return false;
    }

    // we're not using terminators yet
    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor;
    }

    return MatchedExpression(node);
}

void Parser::ConstructAST(const ParseNode& node) {
    if (node.rule != Rule::Artifact) {
        LogErr("Something went very wrong.");
        return;
    }

    if (node.IsLeaf()) {
        LogErr("Empty module, no AST can be constructed");
        return;
    }

    syntax_tree = std::make_unique<Module>(node.tokens[0].text);

    const auto root = dynamic_cast<Module*>(syntax_tree.get());
    for (const auto& n : node.branches) {
        switch (n->rule) {
        case Rule::Term:
        case Rule::Factor:
            root->AddChild<BinaryExpr>(*n);
        default:
            break;
        }
    }
    int x = 1;
}

bool Parser::MatchedExpression(ParseNode& node) {
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
bool Parser::MatchedPrimary(ParseNode& node) {
    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor;
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
bool Parser::MatchedUnary(ParseNode& node) {
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
bool Parser::MatchedFactor(ParseNode& node) {
    return MatchedBinaryExpr(node, IsFactorOp, &Parser::MatchedUnary, Rule::Factor);
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
bool Parser::MatchedTerm(ParseNode& node) {
    return MatchedBinaryExpr(node, IsTermOp, &Parser::MatchedFactor, Rule::Term);
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

// comparison = term ( ('>' | '>=' | '<' | '<=') term)*
bool Parser::MatchedComparison(ParseNode& node) {
    return MatchedBinaryExpr(node, IsComparisonOp, &Parser::MatchedTerm, Rule::Comparison);
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

// equality   = comparison (  ('!=' | '==') comparison)*
bool Parser::MatchedEquality(ParseNode& node) {
    return MatchedBinaryExpr(node, IsEqualityOp, &Parser::MatchedComparison, Rule::Equality);
}

bool Parser::MatchedBinaryExpr(
    ParseNode&           node,
    const OpCheckerFnPtr is_valid_operator,
    const MatcherFnPtr   matched_operand,
    const Rule           rule
) {
    if (not(this->*matched_operand)(node)) {
        return false;
    }

    if (not is_valid_operator(CurrentToken().type)) {
        return true;
    }

    auto& binary_expr = node.NewBranch(rule);
    AddCycledTokenTo(binary_expr);

    // LHS matched, so we need to make it a child of this expr
    binary_expr.AcquireBranchOf(node, node.branches.size() - 2);
    const auto expr_index = node.branches.size() - 1;

    if (not(this->*matched_operand)(node)) {
        LogErr("Expected expression");
        return false;
    }

    bool ret = true;
    while (is_valid_operator(CurrentToken().type)) {
        AddCycledTokenTo(binary_expr);

        if (not(this->*matched_operand)(node)) {
            LogErr("Incomplete expression");
            binary_expr.rule = Rule::Mistake;

            ret = false;
        }
    }
    binary_expr.AcquireBranchesOf(node, expr_index + 1);

    return ret;
}
}  // namespace sigil