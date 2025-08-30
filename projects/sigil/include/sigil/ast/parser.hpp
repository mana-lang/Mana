#pragma once

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/token.hpp>

#include <mana/literals.hpp>

#include <vector>

namespace sigil {
namespace ml = mana::literals;

class Parser {
    using ASTNode = std::unique_ptr<ast::Node>;

    TokenStream tokens;
    ml::i64     cursor;
    ParseNode   parse_tree;
    ASTNode     syntax_tree;

public:
    explicit Parser(const TokenStream&& tokens);
    explicit Parser(const TokenStream& tokens);

    SIGIL_NODISCARD bool Parse();

    SIGIL_NODISCARD auto ViewParseTree() const -> const ParseNode&;
    SIGIL_NODISCARD auto ViewTokens() const -> const TokenStream&;
    SIGIL_NODISCARD auto ViewAST() const -> ast::Node*;

    void PrintParseTree() const;
    void EmitParseTree(std::string_view file_name) const;

    SIGIL_NODISCARD std::string EmitParseTree() const;

private:
    SIGIL_NODISCARD std::string EmitParseTree(const ParseNode& node, std::string prepend = "") const;

    SIGIL_NODISCARD const Token& CurrentToken() const;
    SIGIL_NODISCARD const Token& PeekNextToken() const;
    SIGIL_NODISCARD const Token& NextToken();
    SIGIL_NODISCARD const Token& GetAndCycleToken();

    bool SkipNewlines();

    // Adds tokens to the given node, up to and including the delimiter
    void AddTokensTo(ParseNode& node, TokenType delimiter);

    // Adds 'count' tokens to the given node
    void AddTokensTo(ParseNode& node, ml::i64 count);
    void AddCurrentTokenTo(ParseNode& node) const;
    void AddCycledTokenTo(ParseNode& node);

    bool ProgressedParseTree(ParseNode& node);

    void ConstructAST(const ParseNode& node);

    // Matchers
    bool MatchedStatement(ParseNode& node);
    bool Expect(bool condition, std::string_view error_message, ParseNode& node);

    bool MatchedDeclaration(ParseNode& node);
    bool MatchedAssignment(ParseNode& node);

    bool MatchedExpression(ParseNode& node);

    bool MatchedElemList(ParseNode& node);
    bool MatchedArrayLiteral(ParseNode& node);
    bool MatchedGrouping(ParseNode& node);
    bool MatchedPrimary(ParseNode& node);
    bool MatchedUnary(ParseNode& node);
    bool MatchedFactor(ParseNode& node);
    bool MatchedTerm(ParseNode& node);
    bool MatchedComparison(ParseNode& node);
    bool MatchedEquality(ParseNode& node);

    using MatcherFnPtr   = bool (Parser::*)(ParseNode&);
    using OpCheckerFnPtr = bool (*)(TokenType);

    SIGIL_NODISCARD bool MatchedBinaryExpr(
        ParseNode&     node,
        OpCheckerFnPtr is_valid_operator,
        MatcherFnPtr   matched_operand,
        ast::Rule      rule
    );
};

}  // namespace sigil