#pragma once

#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/token.hpp>

#include <mana/literals.hpp>

namespace sigil {
namespace ml = mana::literals;

class Parser {
    using ASTNode     = std::unique_ptr<ast::Node>;
    using TokenStream = std::vector<Token>;

    TokenStream tokens;
    ml::i64     cursor;
    ParseNode   parse_tree;

    std::unique_ptr<ast::Artifact> syntax_tree;

public:
    explicit Parser(const TokenStream&& tokens);
    explicit Parser(const TokenStream& tokens);

    SIGIL_NODISCARD bool Parse();

    SIGIL_NODISCARD const ParseNode&   ViewParseTree() const;
    SIGIL_NODISCARD const TokenStream& ViewTokenStream() const;
    SIGIL_NODISCARD ast::Node*         ViewAST() const;

    void PrintParseTree() const;
    void EmitParseTree(std::string_view file_name) const;

    SIGIL_NODISCARD std::string EmitParseTree() const;

private:
    SIGIL_NODISCARD std::string EmitParseTree(const ParseNode& node, std::string prepend = "") const;

    SIGIL_NODISCARD Token CurrentToken() const;
    SIGIL_NODISCARD Token PeekNextToken() const;
    SIGIL_NODISCARD Token NextToken();
    SIGIL_NODISCARD Token GetAndCycleToken();

    bool SkipNewlines();

    // Adds tokens to the given node, up to and including the delimiter
    void AddTokensTo(ParseNode& node, TokenType delimiter);

    // Adds 'count' tokens to the given node
    void AddTokensTo(ParseNode& node, ml::i64 count);
    void AddCurrentTokenTo(ParseNode& node) const;
    void AddCycledTokenTo(ParseNode& node);

    bool ProgressedParseTree(ParseNode& node);

    void ConstructAST(const ParseNode& node);

    /// Verifies a required parsing condition and records a recoverable syntax error.
    ///
    /// If `condition` is false, logs an error at the current token, marks `node` as
    /// `Rule::Mistake`. On success, has no side effects.
    ///
    /// @param condition  Condition that must hold for parsing to proceed.
    /// @param node       Parse node to mark as `Rule::Mistake` on failure.
    /// @param error_message  Diagnostic message describing the expected syntax.
    /// @return `true` if the condition holds; `false` otherwise.
    bool Expect(bool condition, ParseNode& node, std::string_view error_message) const;

    // Matchers
    bool MatchedStatement(ParseNode& node);

    bool MatchedScope(ParseNode& node);

    bool MatchedIfBlock(ParseNode& node);
    bool MatchedIfTail(ParseNode& node);

    bool MatchedDeclaration(ParseNode& node);
    bool MatchedAssignment(ParseNode& node);

    bool MatchedExpression(ParseNode& node);
    bool MatchedLogical(ParseNode& node);
    bool MatchedEquality(ParseNode& node);
    bool MatchedComparison(ParseNode& node);
    bool MatchedTerm(ParseNode& node);
    bool MatchedFactor(ParseNode& node);

    bool MatchedElemList(ParseNode& node);
    bool MatchedArrayLiteral(ParseNode& node);
    bool MatchedGrouping(ParseNode& node);
    bool MatchedPrimary(ParseNode& node);
    bool MatchedUnary(ParseNode& node);

    using MatcherFnPtr   = bool (Parser::*)(ParseNode&);
    using OpCheckerFnPtr = bool (*)(TokenType);

    SIGIL_NODISCARD bool MatchedBinaryExpr(
        ParseNode&     node,
        OpCheckerFnPtr is_valid_operator,
        MatcherFnPtr   matched_operand,
        ast::Rule      rule
    );
};
} // namespace sigil
