#pragma once

#include <sigil/ast/parse_tree.hpp>
#include <sigil/ast/token.hpp>
#include <sigil/core/type_aliases.hpp>

#include <vector>

namespace sigil {
struct TokenRange {
    i64 breadth;
    i64 offset;
};

class Parser {
    TokenStream tokens_;
    i64         cursor_;
    ast::Node   ast_;

public:
    explicit Parser(const TokenStream&& tokens);
    explicit Parser(const TokenStream& tokens);

    SIGIL_NODISCARD bool Parse();

    SIGIL_NODISCARD auto ViewAST() const -> const ast::Node&;
    SIGIL_NODISCARD auto ViewTokens() const -> const TokenStream&;

    void PrintAST() const;
    void EmitAST(std::string_view file_name = "Mana.ast") const;

private:
    SIGIL_NODISCARD std::string EmitAST(const ast::Node& root, std::string prepend = "") const;

    SIGIL_NODISCARD auto CurrentToken() const -> const Token&;
    SIGIL_NODISCARD auto PeekNextToken() const -> const Token&;
    SIGIL_NODISCARD auto NextToken() -> const Token&;
    SIGIL_NODISCARD auto GetAndCycleToken() -> const Token&;

    void AddTokensTo(ast::Node& node, TokenType delimiter);
    void AddTokensTo(ast::Node& node, i64 count);
    void AddCurrentTokenTo(ast::Node& node) const;
    void AddCycledTokenTo(ast::Node& node);

    void TransmitTokens(TokenStream& from, TokenStream& to) const;
    void TransmitTokens(ast::Node& from, ast::Node& to, TokenRange range) const;

    bool ProgressedAST(ast::Node& node);

    // Matchers
    SIGIL_NODISCARD bool MatchedExpression(ast::Node& node);

    SIGIL_NODISCARD bool MatchedPrimary(ast::Node& node);
    SIGIL_NODISCARD bool MatchedUnary(ast::Node& node);
    SIGIL_NODISCARD bool MatchedFactor(ast::Node& node);
    SIGIL_NODISCARD bool MatchedTerm(ast::Node& node);
    SIGIL_NODISCARD bool MatchedComparison(ast::Node& node);
    SIGIL_NODISCARD bool MatchedEquality(ast::Node& node);

    using MatcherFnPtr   = bool   (Parser::*)(ast::Node&);
    using OpCheckerFnPtr = bool (*)(TokenType);
    SIGIL_NODISCARD bool          MatchedBinaryExpr(
                 ast::Node&     node,
                 OpCheckerFnPtr is_valid_operator,
                 MatcherFnPtr   matched_operand,
                 ast::Rule      rule
             );
};

}  // namespace sigil