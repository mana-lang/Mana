#pragma once

#include <salem/core/type_aliases.hpp>
#include <salem/frontend/token.hpp>
#include <salem/frontend/ast.hpp>

#include <vector>

namespace salem {
struct token_range {
    usize size_;
    usize offset_;
};

// NOTE: match_ functions all assume that the initial token(s)
// for their rule has already been matched.
// progress_ast() is where this matching process starts
class parser {
private:
    token_stream tokens_;
    ast::node    ast_;
    usize        cursor_;

public:
    explicit parser(const token_stream&& tokens);
    bool     parse();

    SALEM_NODISCARD auto view_ast() const -> const ast::node&;
    SALEM_NODISCARD auto view_tokens() const -> const token_stream&;

    void print_ast() const;
    void print_ast(const ast::node& root, std::string prepend = "") const;

private:
    SALEM_NODISCARD bool is_primitive(token_type token) const;

    SALEM_NODISCARD auto peek_token() const -> const token&;
    SALEM_NODISCARD auto current_token() const -> const token&;

    // advances token cursor
    SALEM_NODISCARD auto next_token() -> const token&;

    bool progress_ast(ast::node& node);

    void add_tokens_until(ast::node& node, token_type delimiter);
    void add_token_to(ast::node& node) const;
    void transmit_tokens(ast::node& from, ast::node& to, token_range range) const;
    void transmit_tokens(ast::node& from, const ast::node& to) const;

    void match_import_decl(ast::node& import_decl);
    void match_import_alias(ast::node& import_alias);
    void match_import_access(ast::node& import_access);

    void match_stmt_init(ast::node& node);
};
} // namespace salem
