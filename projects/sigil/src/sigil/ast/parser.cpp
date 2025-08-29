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
    : tokens {std::move(tokens)}
    , cursor {}
    , parse_tree {Rule::Undefined} {}

Parser::Parser(const TokenStream& tokens)
    : tokens {tokens}
    , cursor {}
    , parse_tree {Rule::Undefined} {}

bool Parser::Parse() {
    const auto& top_token = tokens.front();

    if (top_token.type != TokenType::_artifact_) {
        Log->error("Improper token stream format. Top-level token was: '{}' instead of "
                   "{}",
                   magic_enum::enum_name(top_token.type),
                   magic_enum::enum_name(TokenType::_artifact_));
        return false;
    }

    parse_tree.rule = Rule::Artifact;
    parse_tree.tokens.push_back(tokens.front());

    cursor = 1;
    while (ProgressedParseTree(parse_tree)) {}

    ConstructAST(parse_tree);

    if (CurrentToken().type != TokenType::Eof) {
        // Log->error("It appears we did not parse the entire file.");
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
    Log->debug("Parse tree for artifact '{}'\n\n{}",
               parse_tree.tokens[0].text,
               EmitParseTree(parse_tree));
}

void Parser::EmitParseTree(const std::string_view file_name) const {
    std::ofstream out {std::string(file_name) + std::string(".ptree")};
    if (not out.is_open()) {
        Log->error("Failed to open file '{}' for writing", file_name);
        return;
    }

    out << EmitParseTree(parse_tree);
    if (out.fail()) {
        Log->error("Failed to write to file '{}'", file_name);
        return;
    }

    Log->info("Emitted parse tree to file '{}'", file_name);
}

std::string Parser::EmitParseTree() const {
    return EmitParseTree(parse_tree);
}

std::string Parser::EmitParseTree(const ParseNode& node, std::string prepend) const {
    std::string ret;
    if (node.rule == Rule::Artifact) {
        ret = fmt::format("[{}] -> {}\n\n",
                          magic_enum::enum_name(node.rule),
                          node.tokens[0].text);

    } else {
        ret.append(fmt::format("{}[{}]\n", prepend, magic_enum::enum_name(node.rule)));

        prepend.append("== ");

        if (not node.tokens.empty()) {
            std::ranges::replace(prepend, '=', '-');

            for (const auto& [type, text, position] : node.tokens) {
                if (type == TokenType::Terminator) {
                    continue;
                }
                ret.append(fmt::format("{} [{}] -> {}\n",
                                       prepend,
                                       magic_enum::enum_name(type),
                                       text));
            }

            std::ranges::replace(prepend, '-', '=');
        }
    }

    if (not node.branches.empty()) {
        for (auto& branch : node.branches) {
            ret.append(EmitParseTree(*branch, prepend));
        }
    }

    if (not node.branches.empty() && node.IsRoot()) {
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

void Parser::AddTokensTo(ParseNode& node, const TokenType delimiter) {
    while (CurrentToken().type != delimiter) {
        node.tokens.push_back(GetAndCycleToken());
    }

    AddCycledTokenTo(node);
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

bool Parser::Expect(const bool             condition,
                    const std::string_view error_message,
                    ParseNode&             node) {
    if (not condition) {
        Log->error("Line {} -> {}", CurrentToken().position.line, error_message);
        node.rule = Rule::Mistake;
        return false;
    }
    return true;
}

bool Parser::ProgressedParseTree(ParseNode& node) {
    // don't process eof (final token) before quitting
    if (cursor + 1 >= tokens.size() - 1) {
        return false;
    }

    // we want to skip over empty lines
    if (CurrentToken().type == TokenType::Terminator) {
        ++cursor;
        return true;
    }

    return MatchedStatement(node);
}

void Parser::ConstructAST(const ParseNode& node) {
    if (node.rule != Rule::Artifact) {
        Log->error("Top-level p-tree node was not 'Artifact' but {}",
                   magic_enum::enum_name(node.rule));
        return;
    }

    if (node.IsLeaf()) {
        Log->error("Empty module, no AST can be constructed");
        return;
    }

    syntax_tree = std::make_unique<Artifact>(node.tokens[0].text);

    const auto root = dynamic_cast<Artifact*>(syntax_tree.get());
    for (const auto& n : node.branches) {
        using enum Rule;

        switch (n->rule) {
        case Equality:
        case Comparison:
        case Term:
        case Factor:
            root->AddChild<BinaryExpr>(*n);
            break;
        case Unary:
            root->AddChild<UnaryExpr>(*n);
            break;
        case ArrayLiteral:
            root->AddChild<ast::ArrayLiteral>(*n);
            break;
        default:
            break;
        }
    }
}

bool Parser::SkipNewlines() {
    bool ret = false;

    // clang-format off
    while (cursor < tokens.size()
           && CurrentToken().type == TokenType::Terminator
           && CurrentToken().text == "\n") {
        ret = true;
        ++cursor;
    }
    // clang-format on

    return ret;
}

// stmt = (decl | assign | expr) TERMINATOR
bool Parser::MatchedStatement(ParseNode& node) {
    auto& stmt {node.NewBranch(Rule::Statement)};

    const bool is_statement = MatchedDeclaration(stmt) || MatchedAssignment(stmt)
                              || MatchedExpression(stmt);

    if (not is_statement) {
        if (stmt.branches.empty()) {
            // if stmt has branches, there may be a Rule::Mistake, so we only pop on dead match
            node.PopBranch();
        }
        return false;
    }

    if (not Expect(CurrentToken().type == TokenType::Terminator,
                   "Expected terminator",
                   stmt)) {
        return true;
    }

    AddCycledTokenTo(stmt);
    return true;
}

// decl = KW_MUT? KW_DATA ID ('=' expr)?
bool Parser::MatchedDeclaration(ParseNode& node) {
    const bool matched_keywords = CurrentToken().type == TokenType::KW_data
                                  || (CurrentToken().type == TokenType::KW_mut
                                      && PeekNextToken().type == TokenType::KW_data);
    if (not matched_keywords) {
        return false;
    }

    auto& decl {node.NewBranch(Rule::Declaration)};
    AddTokensTo(decl, TokenType::KW_data);

    if (not Expect(CurrentToken().type == TokenType::Identifier,
                   "Expected identifier",
                   decl)) {
        return true;
    }
    AddCycledTokenTo(decl);

    // declaration without initialisation
    if (CurrentToken().type == TokenType::Terminator) {
        return true;
    }

    if (not Expect(CurrentToken().type == TokenType::Op_Assign, "Expected '='", decl)) {
        return true;
    }
    AddCycledTokenTo(decl);

    Expect(MatchedExpression(decl), "Expected expression", decl);
    return true;
}

bool Parser::MatchedAssignment(ParseNode& node) {
    if (CurrentToken().type != TokenType::Identifier
        || PeekNextToken().type != TokenType::Op_Assign) {
        return false;
    }

    auto& assignment {node.NewBranch(Rule::Assignment)};
    AddTokensTo(assignment, TokenType::Op_Assign);

    Expect(MatchedExpression(assignment), "Expected expression", assignment);
    return true;
}

bool Parser::MatchedExpression(ParseNode& node) {
    return MatchedEquality(node);
}

// elem_list = expr (',' expr)* (',')?  ;
bool Parser::MatchedElemList(ParseNode& node) {
    using enum TokenType;

    auto& elem_list {node.NewBranch()};
    elem_list.rule = Rule::ElemList;

    SkipNewlines();

    if (not MatchedExpression(elem_list)) {
        node.PopBranch();
        return false;
    }

    // trailing comma allowed
    while (CurrentToken().type == Op_Comma) {
        AddCycledTokenTo(elem_list);  // ','

        SkipNewlines();

        if (not MatchedExpression(elem_list)) {
            break;
        }

        SkipNewlines();
    }

    return true;
}

// array_literal = '[' elem_list? ']'   ;
bool Parser::MatchedArrayLiteral(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_BracketLeft) {
        return false;
    }
    auto& array_literal {node.NewBranch()};
    array_literal.rule = Rule::ArrayLiteral;
    AddCycledTokenTo(array_literal);  // '['

    // Allow [\n] etc.
    SkipNewlines();

    // []
    if (CurrentToken().type == TokenType::Op_BracketRight) {
        AddCycledTokenTo(array_literal);  // ']'
        return true;
    }

    if (not Expect(MatchedElemList(array_literal), "Expected elem list", array_literal)) {
        return true;
    }

    // [1, 2, 3,]
    if (CurrentToken().type == TokenType::Op_BracketRight) {
        AddCycledTokenTo(array_literal);  // ']'
        return true;
    }

    SkipNewlines();

    Expect(false, "Expected ']'", array_literal);

    return true;
}

bool Parser::MatchedGrouping(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_ParenLeft) {
        return false;
    }
    auto& grouping = node.NewBranch();
    grouping.rule  = Rule::Grouping;
    AddCycledTokenTo(grouping);

    if (not Expect(MatchedExpression(grouping), "Expected expression", grouping)) {
        return true;
    }

    if (not Expect(grouping.branches.size() == 1,
                   "Grouping may not contain more than one expression",
                   grouping)) {
        return true;
    }

    if (not Expect(CurrentToken().type == TokenType::Op_ParenRight,
                   "Expected ')'",
                   grouping)) {
        return true;
    }

    AddCycledTokenTo(grouping);
    Expect(grouping.branches.size() == 1,
           "Grouping may not contain more than one expression",
           grouping);

    return true;
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

// primary  = grouping | array_literal | literal | ID
// grouping = "(" expr ")"
bool Parser::MatchedPrimary(ParseNode& node) {
    // i'm pretty sure this was an uncaught bug
    // if (CurrentToken().type == TokenType::Terminator) {
    //     ++cursor;
    //     return true;
    // }

    if (MatchedGrouping(node)) {
        return true;
    }

    if (MatchedArrayLiteral(node)) {
        return true;
    }

    if (IsLiteral(CurrentToken().type)) {
        auto& primary {node.NewBranch()};
        primary.rule = Rule::Literal;
        AddCycledTokenTo(primary);
        return true;
    }

    return false;
}

// unary = ("-" | "!") unary | primary
bool Parser::MatchedUnary(ParseNode& node) {
    switch (CurrentToken().type) {
        using enum TokenType;

    case Op_Minus:
    case Op_LogicalNot: {
        auto& unary {node.NewBranch(Rule::Unary)};
        AddCycledTokenTo(unary);

        Expect(MatchedUnary(unary), "Expected resolution into primary expression", unary);
        return true;
    }

    default:
        return MatchedPrimary(node);
    }
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

bool Parser::MatchedBinaryExpr(ParseNode&           node,
                               const OpCheckerFnPtr is_valid_operator,
                               const MatcherFnPtr   matched_operand,
                               const Rule           rule) {
    if (not(this->*matched_operand)(node)) {
        return false;
    }

    // by this point, we've already created a fresh primary node,
    // so from here on out we don't want to return false and risk destroing AST progress
    // or double parsing
    if (not is_valid_operator(CurrentToken().type)) {
        return true;  // if there's no operator following, this is just a primary
    }

    auto& binary_expr = node.NewBranch(rule);
    AddCycledTokenTo(binary_expr);

    // LHS matched, so we need to make it a child of this expr
    const auto lhs_index = node.branches.size() - 2;
    binary_expr.AcquireBranchOf(node, lhs_index);

    // we need to store the index early, as branches may be acquired before we add the rhs
    const auto rhs_index = node.branches.size() - 1;

    if (not Expect((this->*matched_operand)(node), "Expected expression", binary_expr)) {
        return true;
    }

    while (is_valid_operator(CurrentToken().type)) {
        AddCycledTokenTo(binary_expr);

        if (not Expect((this->*matched_operand)(node), "Expected expression", binary_expr)) {
            return true;
        }
    }


    binary_expr.AcquireBranchesOf(node, rhs_index + 1);

    Expect(binary_expr.branches.size() >= 2, "Expected more operands in binary expression", binary_expr);

    return true;
}
}  // namespace sigil