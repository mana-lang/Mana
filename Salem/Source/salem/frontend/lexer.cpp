#include <salem/frontend/lexer.hpp>
#include <salem/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <fstream>
#include <unordered_map>

namespace salem {
// all contextual symbols (e.g. '-' unary vs. subtraction) should be handled before
// the operators method, which crystallizes them into regular operators
// where the hell is the obj file for this
lexer::lexer()
    : cursor_(0)
      , line_number_() {}

void lexer::tokenize_line(const std::string_view current_line) {
    ++line_number_;

    if (current_line.empty()) {
        return;
    }
    cursor_ = 0;

    const auto line_size = static_cast<i64>(current_line.size());
    while (cursor_ < line_size) {
        if (is_comment(current_line[cursor_])) {
            break;
        }

        if (is_whitespace(current_line[cursor_])) {
            ++cursor_;
            continue;
        }

        if (lex_numbers(current_line)) {
            continue;
        }

        if (lex_identifiers(current_line)) {
            continue;
        }

        if (lex_operators(current_line)) {
            continue;
        }

        lex_unknown(current_line);
    }

    add_token(token_type::Newline, "\n");
}

void lexer::add_token(const token_type type, std::string&& text) {
    token_stream_.emplace_back(
        type,
        std::move(text),
        text_position{line_number_, cursor_ + 1}
    );
}

void lexer::add_token(token_type type, std::string& text) {
    token_stream_.emplace_back(
        type,
        text,
        text_position{line_number_, cursor_ + 1}
    );
}

void lexer::add_eof() {
    token_stream_.emplace_back(
        EOF_TOKEN.type_,
        EOF_TOKEN.text_,
        text_position{line_number_, cursor_ + 2} // display EOF tokens as being
                                                 // out of bounds of file contents
    );
}

bool lexer::tokenize_file(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (not file.is_open()) {
        log(log_level::Error, "Failed to open file at '{}'",
            file_path.string());
        return false;
    }

    token_stream_.clear();
    line_number_ = -1;
    cursor_ = -1;

    add_token(
        token_type::_module_,
        file_path.filename().replace_extension("").string()
    );

    line_number_ = 0;
    cursor_ = 0;

    //token_stream_.emplace_back( text_position(-1, -1));

    std::string current_line;
    while (std::getline(file, current_line)) {
        current_line.push_back('\n'); // reinsert delimiter for seek operations
        tokenize_line(current_line);
    }

    add_eof();
    return true;
}

void lexer::print_tokens() const {
    if (token_stream_.empty()) {
        log(log_level::Error,
            "Lexer token print requested, but token stream was empty.");
        return;
    }


    log(log_level::Debug, "--- Printing Token Stream ---\n");

    for (const auto& [type, contents, position] : token_stream_) {
        if (type == token_type::Newline) {
            log(log_level::Info, "[L: {} | C: {}] {}: \\n",
                position.line, position.column, magic_enum::enum_name(type));
            continue;
        }
        log(log_level::Info, "[L: {} | C: {}] {}: {}",
            position.line, position.column, magic_enum::enum_name(type),
            contents);
    }
    log(log_level::Debug, "End of Stream.\n");
}

void lexer::clear() {
    token_stream_.clear();
}

auto lexer::relinquish_tokens() -> std::vector<token>&& {
    return std::move(token_stream_);
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
bool lexer::lex_identifiers(const std::string_view current_line) {
    if (current_line[cursor_] == '_' || std::isalpha(current_line[cursor_])) {
        std::string buffer;
        while (current_line[cursor_] == '_' || std::isalnum(
            current_line[cursor_])) {
            buffer.push_back(current_line[cursor_++]);
        }

        if (not match_keyword(buffer)) {
            add_token(token_type::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
bool lexer::lex_strings(std::string_view current_line) {
    std::string buffer;
    buffer.push_back(current_line[cursor_]);

    token_type literal_type;
    switch (current_line[cursor_]) {
    case '\"':
        literal_type = token_type::Lit_String;
        break;
    case '\'':
        literal_type = token_type::Lit_Char;
        break;
    default:
        log(log_level::Error, "Erroneous call to string lexer");
        add_token(token_type::Unknown, std::move(buffer));
        return false;
    }

    while (true) {
        if (static_cast<usize>(++cursor_) >= current_line.size()) {
            // next token should always be newline or string literal
            log(log_level::Warn, "Unexpected EOF while lexing string literal");
            add_token(token_type::Unknown, std::move(buffer));
            add_eof();
            return false;
        }

        const char current_char = current_line[cursor_];

        if (current_char == '\n' || (
                current_char == '\\' &&
                current_line[cursor_ + 1] == 'n'
            )       // strings must close on the line they're started
        ) { return false; }

        buffer.push_back(current_char);

        if (current_char == '\'' || current_char == '\"') {
            ++cursor_;
            break;
        }
    }

    add_token(literal_type, std::move(buffer));
    return true;
}

bool lexer::match_keyword(std::string& ident_buffer) {
    using TokenMap = std::unordered_map<std::string, token_type>;
    using enum token_type;
    static const TokenMap keyword_map = {
        {"i8", KW_i8},
        {"i16", KW_i16},
        {"i32", KW_i32},
        {"i64", KW_i64},
        {"i128", KW_i128},

        {"u8", KW_u8},
        {"u16", KW_u16},
        {"u32", KW_u32},
        {"u64", KW_u64},
        {"u128", KW_u128},

        {"f32", KW_f32},
        {"f64", KW_f64},

        {"byte", KW_byte},
        {"char", KW_char},
        {"string", KW_string},
        {"bool", KW_bool},
        {"void", KW_void},

        {"data", KW_data},
        {"fn", KW_fn},
        {"mut", KW_mut},
        {"raw", KW_raw},
        {"const", KW_const},
        {"override", KW_override},

        {"pack", KW_pack},
        {"struct", KW_struct},
        {"enum", KW_enum},
        {"generic", KW_generic},

        {"module", KW_module},
        {"public", KW_public},
        {"private", KW_private},
        {"import", KW_import},
        {"as", KW_as},

        {"return", KW_return},
        {"true", KW_true},
        {"false", KW_false},
        {"if", KW_if},
        {"else", KW_else},
        {"match", KW_match},

        {"loop", KW_loop},
        {"while", KW_while},
        {"for", KW_for},
        {"break", KW_break},
        {"skip", KW_skip},

        {"and", Op_LogicalAnd},
        {"or", Op_LogicalOr},
        {"not", Op_LogicalNot},
    };

    if (const auto keyword = keyword_map.find(ident_buffer);
        keyword != keyword_map.end()
    ) {
        add_token(keyword->second, std::move(ident_buffer));
        return true;
    }
    return false;
}

void lexer::lex_unknown(const std::string_view current_line) {
    std::string buffer;
    while (!is_whitespace(current_line[cursor_])) {
        buffer.push_back(current_line[cursor_++]);
    }

    if (not buffer.empty()) {
        add_token(token_type::Unknown, std::move(buffer));
    }
}

bool lexer::is_whitespace(const char c) const {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

bool lexer::is_comment(const char c) const {
    return c == '#';
}

// NUMBER = INT | FLOAT
bool lexer::lex_numbers(const std::string_view current_line) {
    const auto current_char = current_line[cursor_];

    const bool is_negative_digit = current_char == '-' && std::isdigit(
        current_line[cursor_ + 1]);
    if (not is_negative_digit
        && not std::isdigit(current_char)) {
        return false;
    }

    // We're guaranteed to have at least a 1-digit number by now,
    // so we can consume the current char
    std::string buffer;
    buffer.push_back(current_char);
    ++cursor_;

    // INT = ^[-?0-9]+
    const auto eat_digits = [&] {
        while (std::isdigit(current_line[cursor_])) {
            buffer.push_back(current_line[cursor_++]);
        }
    };

    eat_digits();

    // FLOAT = INT.[0-9]+
    // if we encounter a dot, it can't be an int
    if (current_line[cursor_] != '.') {
        add_token(token_type::Lit_Int, std::move(buffer));
        return true;
    }

    // eat dot before progressing
    buffer.push_back(current_line[cursor_++]);

    eat_digits();

    add_token(token_type::Lit_Float, std::move(buffer));
    return true;
}

bool lexer::lex_operators(const std::string_view current_line) {
    const auto current_char = current_line[cursor_];
    const auto next_char = current_line[cursor_ + 1];
    token_type token_type;

    switch (current_char) {
        using enum token_type;
    case '=':
        if (next_char == '=') {
            token_type = Op_Equality; // ==
            break;
        }
        token_type = Op_Assign;
        break;
    case '+':
        token_type = Op_Plus;
        break;
    case '-':
        if (next_char == '>') {
            token_type = Op_Arrow; // ->
            break;
        }
        token_type = Op_Minus;
        break;
    case '*':
        token_type = Op_Asterisk;
        break;
    case '/':
        token_type = Op_FwdSlash;
        break;
    case ':':
        if (next_char == ':') {
            token_type = Op_ModuleElementAccess; // ::
            break;
        }
        token_type = Op_Colon;
        break;
    case ',':
        token_type = Op_Comma;
        break;
    case '{':
        token_type = Op_BraceLeft;
        break;
    case '}':
        token_type = Op_BraceRight;
        break;
    case '(':
        token_type = Op_ParenLeft;
        break;
    case ')':
        token_type = Op_ParenRight;
        break;
    case '[':
        token_type = Op_BracketLeft;
        break;
    case ']':
        token_type = Op_BracketRight;
        break;
    case '.':
        token_type = Op_Period;
        break;
    case '!':
        if (next_char == '=') {
            token_type = Op_NotEqual; // !=
            break;
        }
        token_type = Op_LogicalNot;
        break;
    case '<':
        if (next_char == '=') {
            token_type = Op_LessEqual; // <=
            break;
        }
        token_type = Op_LessThan;
        break;
    case '>':
        if (next_char == '=') {
            token_type = Op_GreaterEqual; // >=
            break;
        }
        token_type = Op_GreaterThan;
        break;
    case '&':
        token_type = Op_ExplicitRef;
        break;
    case '~':
        token_type = Op_ExplicitMove;
        break;
    case '$':
        token_type = Op_ExplicitCopy;
        break;
    case '\"':
    case '\'':
        return lex_strings(current_line);

    default:
        return false;
    }

    std::string buffer(1, current_char);

    switch (token_type) {
        using enum token_type;

    case Op_ModuleElementAccess:
    case Op_Equality:
    case Op_NotEqual:
    case Op_LessEqual:
    case Op_GreaterEqual:
    case Op_Arrow:
        buffer.push_back(next_char);
        ++cursor_;
        break;
    default: break;
    }

    add_token(token_type, std::move(buffer));
    ++cursor_;

    return true;
}
} // namespace salem
