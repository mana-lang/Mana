#include <magic_enum/magic_enum.hpp>
#include <salem/frontend/parser.hpp>
#include <salem/core/logger.hpp>

namespace salem {

using namespace ast;

parser::parser(const token_stream&& tokens)
    : tokens_(tokens)
      , ast_({})
      , cursor_(0) {}

bool parser::parse() {
    const auto top_token = tokens_.front();
    if (top_token.type_ != token_type::_module_) {
        log(log_level::Error,
            "Improper token stream format. Top-level token was: '{}' instead of '_module_'",
            magic_enum::enum_name(top_token.type_)
        );
        return false;
    }

    ast_.rule_ = rule::Module;
    ast_.tokens_.push_back(tokens_.front());

    cursor_ = 0;
    while (progress_ast(ast_)) {}

    return true;
}

auto parser::view_ast() const -> const node& {
    return ast_;
}

auto parser::view_tokens() const -> const token_stream& {
    return tokens_;
}

bool parser::is_primitive(const token_type token) const {
    switch (token) {
        using enum token_type;

    case KW_i32:
    case KW_i64:

    case KW_u32:
    case KW_u64:

    case KW_f32:
    case KW_f64:

    case KW_char:
    case KW_string:
    case KW_byte:
    case KW_void:
        return true;

    default:
        return false;
    }
}

auto parser::peek_token() const -> const token& {
    return tokens_[cursor_ + 1];
}

auto parser::next_token() -> const token& {
    return tokens_[++cursor_];
}

auto parser::current_token() const -> const token& {
    return tokens_[cursor_];
}

bool parser::progress_ast(node& node) {
    bool result = true;

    // quit without processing eof
    if (cursor_ + 1 >= tokens_.size() - 1) {
        result = false;
    }

    switch (next_token()) {
        using enum token_type;

    case KW_import:
        match_import_decl(*node.new_branch(rule::Decl_Import));
        break;

    default:
        add_token_to(node);
    }

    return result;
}

void parser::add_tokens_until(node& node, const token_type delimiter) {
    while (current_token().type_ != delimiter) {
        if (not progress_ast(node)) {
            return;
        }
    }
}

void parser::add_token_to(node& node) {
    if (cursor_ < tokens_.size()) {
        node.tokens_.push_back(current_token());
    }
}

/// TODO: Make transmit functions take reference to token_stream instead
void parser::transmit_tokens(node& sender, node& receiver,
                            token_range range) {
    const auto [size, offset] = range;

    if (size + offset > sender.tokens_.size()) {
        log(
            log_level::Error,
            "void parser::token_transmit(node& sender, node& receiver, token_range range)"
        );
        return;
    }

    for (usize i = 0; i < size; ++i) {
        receiver.tokens_.emplace_back(sender.tokens_[offset]);

    }
}

void parser::transmit_tokens(node& sender, node& receiver) {
    auto receive = receiver.tokens_;
    for (const auto& send : sender.tokens_) {
        receive.push_back(send);
    }

    sender.tokens_.clear();
}

// import_decl   ::= import_module import_access import_alias?
void parser::match_import_decl(node& import_decl) {
    add_token_to(import_decl);

    if (next_token().type_ != token_type::Identifier) {
        log(log_level::Error,
            "Incorrect 'import' declaration. Expected 'Identifier', got '{}' ({})",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
        );
    }

    match_import_access(*import_decl.new_branch(rule::Import_Access));

    if (next_token().type_ == token_type::KW_as) {
        match_import_alias(*import_decl.new_branch(rule::Import_Alias));
    }

    if (next_token().type_ != token_type::Terminator) {
        log(log_level::Error,
            "Incorrect 'import' declaration. Expected 'Terminator', got '{}' ({})",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
        );
    }



    while (true) {
        switch (next_token().type_) {
            using enum token_type;

        case KW_as:

        case Identifier:
        case Op_Period:
            add_token_to(import_decl);
            continue;
        default:
                break;
        }
        break;
    }
}

// import_alias  ::= (KW_AS (IDENTIFIER import_access | OP_STAR))
void parser::match_import_alias(node& import_alias) {
    // 'as' keyword
    add_token_to(import_alias);

    using enum token_type;

    if (peek_token().type_ == Op_Star) {
        ++cursor_;
        add_token_to(import_alias);
        return;
    }

    // next token is guaranteed to not be star
    if (next_token().type_ != Identifier) {
        log(log_level::Error,
            "Incorrect import alias: Expected 'Identifier', got '{}' ({})",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
        );
    }

    match_import_access(*import_alias.new_branch(rule::Import_Alias));
}

// import_access ::= (OP_PERIOD IDENTIFIER)*
void parser::match_import_access(node& import_access) {
    add_token_to(import_access);

    while (true) {
        switch (next_token().type_) {
            using enum token_type;

        case Identifier:
        case Op_Period:
            add_token_to(import_access);
            continue;

        default:
            break;
        }
        break;
    }
    // rewind cursor by 1 as current token
    // is unknown, and may be consumed by anything
    --cursor_;
}

void parser::match_terminator(node& terminator) {
    add_token_to(terminator);


}

void parser::match_stmt_init(node& node) {
    switch (tokens_[cursor_].type_) {
        using enum token_type;

    case KW_import:
        match_import_decl(node);
        break;
    default:
        log(log_level::Error, "Cannot initiate statement with {}", tokens_[cursor_].text_);

    }
}

} // namespace salem
