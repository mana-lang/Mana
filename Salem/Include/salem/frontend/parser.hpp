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

class parser {
private:
    token_stream tokens_;
    ast::node ast_;
    usize cursor_;

public:
    explicit parser(const token_stream&& tokens);
    bool parse();

    /// TODO: fix ownership issues with ast nodes
    SALEM_NODISCARD auto view_ast() const -> const ast::node&;
    SALEM_NODISCARD auto view_tokens() const -> const token_stream&;

private:
    bool progress_ast(ast::node& node);

    void token_add_until(token_type delimiter, ast::node& node);
    void token_add(ast::node& node);
    void token_transmit(ast::node& from, ast::node& to, token_range range);
    void token_transmit(ast::node& from, ast::node& to);

};

} // namespace salem