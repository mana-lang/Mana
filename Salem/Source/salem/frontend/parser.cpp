#include <magic_enum/magic_enum.hpp>
#include <salem/frontend/parser.hpp>
#include <salem/core/logger.hpp>

namespace salem {
using namespace ast;

parser::parser(const token_stream&& tokens)
    : tokens_(tokens),
      ast_({}),
      cursor_(0) {}

bool parser::parse() {
    const auto top_token = tokens_.front();
    if (top_token.type_ != token_type::_module_) {
        log(
            log_level::Error,
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

void parser::print_ast() const {
    log(
        log_level::Info,
        "Printing AST for module \'{}\'",
        ast_.tokens_[0].text_
       );
    print_ast(ast_);
}

void parser::print_ast(const node& root, std::string prepend) const {
    log(log_level::Info, "{}[{}]", prepend, magic_enum::enum_name(root.rule_));

    prepend.append("==== ");

    if (not root.branches_.empty()) {
        for (const auto& n : root.branches_) {
            print_ast(*n, prepend);
        }
    }

    std::ranges::replace(prepend, '=', '-');

    for (const auto& t : root.tokens_) {
        if (t.type_ == token_type::Terminator) {
            continue;
        }
        log(
            log_level::Info,
            "{} [{}] -> {}",
            prepend,
            magic_enum::enum_name(t.type_),
            t.text_
           );
    }

    if (root.tokens_.size() > 1) {
        log(log_level::Info, "");
    }
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

    switch (next_token().type_) {
        using enum token_type;

    case KW_import:
        match_import_decl(*node.new_branch());
        break;

    default:
        add_current_token_to(node);
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

void parser::add_current_token_to(node& node) const {
    if (cursor_ < tokens_.size()) {
        node.tokens_.push_back(current_token());
    }
}

/// TODO: Make transmit functions take reference to token_stream instead
void parser::transmit_tokens(node&       sender,
                             node&       receiver,
                             token_range range) const {
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

void parser::transmit_tokens(node& sender, const node& receiver) const {
    auto receive = receiver.tokens_;
    for (const auto& send : sender.tokens_) {
        receive.push_back(send);
    }

    sender.tokens_.clear();
}

void parser::match_decl(node& decl) {
    const auto token = current_token().type_;
    switch (token) {
        using enum token_type;

    case KW_import:
        match_import_decl(decl);
        break;
    default:
        log(log_level::Error,
            "Cannot initiate statement with {} [{}]",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
           );

    }
}

// import_decl   ::= KW_IMPORT IDENTIFIER import_access import_alias?
void parser::match_import_decl(node& import_decl) {
    import_decl.rule_ = rule::Import_Decl;
    add_current_token_to(import_decl);

    if (next_token().type_ != token_type::Identifier) {
        log(log_level::Error,
            "Incorrect 'import' declaration. Expected 'Identifier', got '{}' ({})",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
           );
    }

    match_import_access(*import_decl.new_branch());

    if (next_token().type_ == token_type::KW_as) {
        match_import_alias(*import_decl.new_branch());
    }

    if (next_token().type_ != token_type::Terminator) {
        log(log_level::Error,
            "Incorrect 'import' declaration. Expected 'Terminator', got '{}' ({})",
            current_token().text_,
            magic_enum::enum_name(current_token().type_)
           );
    }
}

// import_alias  ::= (KW_AS (IDENTIFIER import_access | OP_STAR))
void parser::match_import_alias(node& import_alias) {
    // 'as' keyword
    add_current_token_to(import_alias);

    using enum token_type;

    while (true) {
        if (peek_token().type_ == Op_Star) {
            ++cursor_;
            add_current_token_to(import_alias);
            return;
        }

        // next token is guaranteed to not be op_star
        if (next_token().type_ != Identifier) {
            log(log_level::Error,
                "Incorrect import alias: Expected 'Identifier', got '{}' ({})",
                current_token().text_,
                magic_enum::enum_name(current_token().type_)
               );
        }

        match_import_access(*import_alias.new_branch());
    }
}

// import_access ::= IDENTIFIER (OP_PERIOD IDENTIFIER)*
void parser::match_import_access(node& import_access) {
    import_access.rule_ = rule::Import_Access;
    add_current_token_to(import_access);

    // might as well unwrap imported modules early
    switch (next_token().type_) {
        using enum token_type;

    case Identifier:
        match_import_access(import_access);
        return;
    case Op_Period:
        match_import_access(*import_access.new_branch());
        return;

    default:
        ;
    }
}

// module_decl ::= access_spec? KW_MODULE IDENTIFIER OP_COLON
void parser::match_module_decl(node& module_decl) {
    module_decl.rule_ = rule::Module_Decl;
    add_current_token_to(module_decl);

    match_access_spec(*module_decl.new_branch());
    if (module_decl.branches_.back()->rule_ != rule::AccessSpec) {
        module_decl.branches_.pop_back();
    }

    if (next_token().type_ != token_type::Identifier) {
        log(log_level::Error,
            // really should implement that error sink already
            "Incorrect module declaration. Expected 'Identifier', got {} [{}]",
            current_token().text_,
            current_token().type_
           );
        module_decl.rule_ = rule::Mistake;
    }

    if (next_token().type_ != token_type::Op_Colon) {
        log(log_level::Error,
            "Incorrect module declaration. Expected ':', got {} [{}]",
            current_token().text_,
            current_token().type_
           );
        module_decl.rule_ = rule::Mistake;
    }
    add_current_token_to(module_decl);
}

// access_spec ::= KW_PUBLIC | KW_PRIVATE
void parser::match_access_spec(node& access_specifier) const {
    access_specifier.rule_ = rule::AccessSpec;

    using enum token_type;
    if (const auto t = current_token().type_;
        t == KW_public
        || t == KW_private) {
        add_current_token_to(access_specifier);
    }
    else {
        access_specifier.rule_ = rule::Mistake;
    }

}

// access_decl ::= access_spec IDENTIFIER? OP_COLON
void parser::match_access_decl(node& access_decl) {
    access_decl.rule_ = rule::AccessDecl;

    match_access_spec(*access_decl.new_branch());
    if (access_decl.branches_.back()->rule_ != rule::AccessSpec) {
        log(log_level::Error,
            "Incorrect access declaration. Expected 'AccessSpec', got {} [{}]",
            current_token().text_,
            current_token().type_
           );
        return;
    }

    if (next_token().type_ == token_type::Identifier) {
        add_current_token_to(access_decl);
    }

    if (next_token().type_ != token_type::Op_Colon) {
        log(log_level::Error,
            "Incorrect access declaration. Expected ':', got {} [{}]",
            current_token().text_,
            current_token().type_
           );
        return;
    }

    add_current_token_to(access_decl);
}
} // namespace salem
