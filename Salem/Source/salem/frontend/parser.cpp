#include <magic_enum/magic_enum.hpp>
#include <salem/frontend/parser.hpp>
#include <salem/core/logger.hpp>

namespace salem {
parser::parser(const std::vector<token>&& tokens)
    : tokens_(tokens)
      , ast_({})
      , cursor_(0) {}

void parser::parse() {
    const auto top_token = tokens_.front().type_;
    if (top_token != token_type::_module_) {
        log(
            log_level::Error,
            "Improper token stream format. Top-level token was: {}",
            magic_enum::enum_name(top_token)
        );
        return;
    }

    ast_.rule_ = ast::rule::Module;
    ast_.tokens_.push_back(tokens_.front());

    cursor_ = 0;
    while (progress_ast(ast_));
}

bool parser::progress_ast(ast::node& node) {
    bool result = true;

    if (cursor_ + 1 >= tokens_.size() - 1) {
        result = false;
    }

    // switch (tokens_[++cursor_].type_) {
    //     using enum token_type;
    //
    // }

    return result;
}

void parser::token_add_until(token_type delimiter, ast::node& node) {}
void parser::token_add(ast::node& node) {}

/// TODO: Make transmit functions take reference to token_stream instead
void parser::token_transmit(ast::node& sender, ast::node& receiver,
                            token_range range) {
    const auto [size, offset] = range;

    if (size + offset > sender.tokens_.size()) {
        log(
            log_level::Error,
            "void parser::token_transmit(ast::node& sender, ast::node& receiver, token_range range)"
        );
        return;
    }

    for (usize i = 0; i < size; ++i) {
        receiver.tokens_.emplace_back(sender.tokens_[offset]);

    }
}

void parser::token_transmit(ast::node& sender, ast::node& receiver) {}
}
