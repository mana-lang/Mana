#pragma once

#include <mana/literals.hpp>
#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parse-tree.hpp>
#include <sigil/ast/token.hpp>

#include <vector>

namespace sigil {
namespace ml = mana::literals;

struct TokenRange {
    ml::i64 breadth;
    ml::i64 offset;
};

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
    void EmitParseTree(std::string_view file_name = "Mana.ast") const;

private:
    SIGIL_NODISCARD std::string EmitParseTree(const ParseNode& root, std::string prepend = "") const;

    SIGIL_NODISCARD auto CurrentToken() const -> const Token&;
    SIGIL_NODISCARD auto PeekNextToken() const -> const Token&;
    SIGIL_NODISCARD auto NextToken() -> const Token&;
    SIGIL_NODISCARD auto GetAndCycleToken() -> const Token&;

    void AddTokensTo(ParseNode& node, TokenType delimiter);
    void AddTokensTo(ParseNode& node, ml::i64 count);
    void AddCurrentTokenTo(ParseNode& node) const;
    void AddCycledTokenTo(ParseNode& node);

    void TransmitTokens(TokenStream& from, TokenStream& to) const;
    void TransmitTokens(ParseNode& from, ParseNode& to, TokenRange range) const;

    bool ProgressedParseTree(ParseNode& node);

    void ConstructAST(const ParseNode& node);

    // Matchers
    SIGIL_NODISCARD bool MatchedExpression(ParseNode& node);

    SIGIL_NODISCARD bool MatchedPrimary(ParseNode& node);
    SIGIL_NODISCARD bool MatchedUnary(ParseNode& node);
    SIGIL_NODISCARD bool MatchedFactor(ParseNode& node);
    SIGIL_NODISCARD bool MatchedTerm(ParseNode& node);
    SIGIL_NODISCARD bool MatchedComparison(ParseNode& node);
    SIGIL_NODISCARD bool MatchedEquality(ParseNode& node);

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