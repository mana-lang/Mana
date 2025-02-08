#pragma once

#include <mana/literals.hpp>
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
    TokenStream    tokens;
    ml::i64        cursor;
    ast::ParseNode parse_tree;

public:
    explicit Parser(const TokenStream&& tokens);
    explicit Parser(const TokenStream& tokens);

    SIGIL_NODISCARD bool Parse();

    SIGIL_NODISCARD auto ViewParseTree() const -> const ast::ParseNode&;
    SIGIL_NODISCARD auto ViewTokens() const -> const TokenStream&;

    void PrintParseTree() const;
    void EmitParseTree(std::string_view file_name = "Mana.ast") const;

private:
    SIGIL_NODISCARD std::string EmitParseTree(const ast::ParseNode& root, std::string prepend = "") const;

    SIGIL_NODISCARD auto CurrentToken() const -> const Token&;
    SIGIL_NODISCARD auto PeekNextToken() const -> const Token&;
    SIGIL_NODISCARD auto NextToken() -> const Token&;
    SIGIL_NODISCARD auto GetAndCycleToken() -> const Token&;

    void AddTokensTo(ast::ParseNode& node, TokenType delimiter);
    void AddTokensTo(ast::ParseNode& node, ml::i64 count);
    void AddCurrentTokenTo(ast::ParseNode& node) const;
    void AddCycledTokenTo(ast::ParseNode& node);

    void TransmitTokens(TokenStream& from, TokenStream& to) const;
    void TransmitTokens(ast::ParseNode& from, ast::ParseNode& to, TokenRange range) const;

    bool ProgressedAST(ast::ParseNode& node);

    // Matchers
    SIGIL_NODISCARD bool MatchedExpression(ast::ParseNode& node);

    SIGIL_NODISCARD bool MatchedPrimary(ast::ParseNode& node);
    SIGIL_NODISCARD bool MatchedUnary(ast::ParseNode& node);
    SIGIL_NODISCARD bool MatchedFactor(ast::ParseNode& node);
    SIGIL_NODISCARD bool MatchedTerm(ast::ParseNode& node);
    SIGIL_NODISCARD bool MatchedComparison(ast::ParseNode& node);
    SIGIL_NODISCARD bool MatchedEquality(ast::ParseNode& node);

    using MatcherFnPtr   = bool (Parser::*)(ast::ParseNode&);
    using OpCheckerFnPtr = bool (*)(TokenType);

    SIGIL_NODISCARD bool MatchedBinaryExpr(
        ast::ParseNode& node,
        OpCheckerFnPtr  is_valid_operator,
        MatcherFnPtr    matched_operand,
        ast::Rule       rule
    );
};

}  // namespace sigil