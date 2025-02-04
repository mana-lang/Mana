#pragma once

#include <hex/ast/parse_tree.hpp>
#include <hex/ast/token.hpp>
#include <hex/core/type_aliases.hpp>

#include <vector>

namespace hex {
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

    HEX_NODISCARD bool Parse();

    HEX_NODISCARD auto ViewAST() const -> const ast::Node&;
    HEX_NODISCARD auto ViewTokens() const -> const TokenStream&;

    void PrintAST() const;
    void EmitAST(std::string_view file_name = "Mana.ast") const;

private:
    HEX_NODISCARD std::string EmitAST(const ast::Node& root, std::string prepend = "") const;

    HEX_NODISCARD auto CurrentToken() const -> const Token&;
    HEX_NODISCARD auto PeekNextToken() const -> const Token&;
    HEX_NODISCARD auto NextToken() -> const Token&;
    HEX_NODISCARD auto GetAndCycleToken() -> const Token&;

    void AddTokensTo(ast::Node& node, TokenType delimiter);
    void AddTokensTo(ast::Node& node, i64 count);
    void AddCurrentTokenTo(ast::Node& node) const;
    void AddCycledTokenTo(ast::Node& node);

    void TransmitTokens(TokenStream& from, TokenStream& to) const;
    void TransmitTokens(ast::Node& from, ast::Node& to, TokenRange range) const;

    bool ProgressedAST(ast::Node& node);

    // Matchers
    HEX_NODISCARD bool MatchedExpression(ast::Node& node);

    HEX_NODISCARD bool MatchedPrimary(ast::Node& node);
    HEX_NODISCARD bool MatchedUnary(ast::Node& node);
    HEX_NODISCARD bool MatchedFactor(ast::Node& node);
    HEX_NODISCARD bool MatchedTerm(ast::Node& node);
    HEX_NODISCARD bool MatchedComparison(ast::Node& node);
    HEX_NODISCARD bool MatchedEquality(ast::Node& node);

    using MatcherFnPtr   = bool   (Parser::*)(ast::Node&);
    using OpCheckerFnPtr = bool (*)(TokenType);
    HEX_NODISCARD bool          MatchedBinaryExpr(
                 ast::Node&     node,
                 OpCheckerFnPtr is_valid_operator,
                 MatcherFnPtr   matched_operand,
                 ast::Rule      rule
             );
};

}  // namespace hex