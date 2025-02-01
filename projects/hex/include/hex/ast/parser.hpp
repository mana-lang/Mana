#pragma once

#include <hex/core/type_aliases.hpp>
#include <hex/ast/token.hpp>
#include <hex/ast/syntax_tree.hpp>

#include <vector>

namespace hex {
struct TokenRange {
  i64 breadth;
  i64 offset;
};

// NOTE: match_ functions all assume that the initial token(s)
// for their rule has already been matched.
// progress_ast() is where this matching process starts
class Parser {
  TokenStream tokens_;
  i64 cursor_;
  ast::Node ast_;

public:
  explicit Parser(const TokenStream&& tokens);
  explicit Parser(const TokenStream& tokens);

  HEX_NODISCARD bool Parse();

  HEX_NODISCARD auto ViewAST() const -> const ast::Node&;
  HEX_NODISCARD auto ViewTokens() const -> const TokenStream&;

  void PrintAST() const;
  void PrintAST(const ast::Node& root, std::string prepend = "") const;

private:
  HEX_NODISCARD bool IsPrimitive(TokenType token_type) const;

  HEX_NODISCARD auto CurrentToken() const -> const Token&;
  HEX_NODISCARD auto PeekNextToken() const -> const Token&;
  HEX_NODISCARD auto NextToken() -> const Token&;
  HEX_NODISCARD auto CycleToken() -> const Token&;

  void AddTokensTo(ast::Node& node, TokenType delimiter);
  void AddTokensTo(ast::Node& node, i64 count);
  void AddCurrentTokenTo(ast::Node& node) const;

  void TransmitTokens(ast::Node& from, ast::Node& to) const;
  void TransmitTokens(ast::Node& from, ast::Node& to, TokenRange range) const;

  bool ProgressedAST(ast::Node& node);


  // Matchers
  // void Match_Statement(ast::Node& node);
  bool Matched_Expression(ast::Node& node);
  bool Matched_Literal(ast::Node& node);
  bool Matched_Unary(ast::Node& node);

};

} // namespace hex