#pragma once

#include <salem/core/type_aliases.hpp>
#include <salem/frontend/token.hpp>
#include <salem/frontend/ast.h>

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
    explicit parser(const std::vector<token>&& tokens);
    void parse();

private:
    bool progress_ast(ast::node& node);

    void token_add_until(token_type delimiter, ast::node& node);
    void token_add(ast::node& node);
    void token_transmit(ast::node& from, ast::node& to, token_range range);
    void token_transmit(ast::node& from, ast::node& to);

};

} // namespace salem