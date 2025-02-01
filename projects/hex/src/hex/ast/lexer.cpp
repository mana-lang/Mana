#include <hex/ast/lexer.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>

namespace hex {

Lexer::Lexer()
    : cursor_ {0}
    , line_number_ {0} {}

void Lexer::TokenizeLine(std::string_view current_line) {
    ++line_number_;

    if (current_line.empty()) {
        return;
    }

    cursor_ = 0;

    const auto line_length = static_cast<i64>(current_line.size());
    while (cursor_ < line_length) {
        if (IsComment(current_line[cursor_])) {
            break;
        }

        if (IsWhitespace(current_line[cursor_])) {
            ++cursor_;
            continue;
        }

        if (LexedNumber(current_line)) {
            continue;
        }

        if (LexedIdentifier(current_line)) {
            continue;
        }

        if (LexedOperator(current_line)) {
            continue;
        }

        LexUnknown(current_line);
    }

    AddToken(TokenType::Terminator, "\n");
}

bool Lexer::Tokenize(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (not file.is_open()) {
        Log(LogLevel::Error, "Failed to open file at '{}'", file_path.string());
        return false;
    }

    token_stream_.clear();

    line_number_ = -1;
    cursor_      = -1;
    AddToken(TokenType::_module_, file_path.filename().replace_extension("").string());

    line_number_ = 0;
    cursor_      = 0;

    std::string current_line;
    while (std::getline(file, current_line)) {
        current_line.push_back('\n');
        TokenizeLine(current_line);
    }

    AddEOF();
    return true;
}

void Lexer::PrintTokens() const {
    if (token_stream_.empty()) {
        Log(LogLevel::Error,
            "Lexer token print requested, but token stream was empty.");
        return;
    }

    Log(LogLevel::Debug, "--- Printing Token Stream ---\n");

    for (const auto& [type, contents, position] : token_stream_) {
        if (type == TokenType::Terminator) {
            Log(LogLevel::Info,
                "[L: {} | C: {}] {}: \\n",
                position.line,
                position.column,
                magic_enum::enum_name(type));
            continue;
        }
        Log(LogLevel::Info,
            "[L: {} | C: {}] {}: \\n",
            position.line,
            position.column,
            magic_enum::enum_name(type),
            contents);
    }
    Log(LogLevel::Debug, "End of token stream.\n");
}

void Lexer::clear() {
    token_stream_.clear();
}

HEX_NODISCARD TokenStream&& Lexer::RelinquishTokens() {
    return std::move(token_stream_);
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
HEX_NODISCARD bool Lexer::LexedIdentifier(const std::string_view line) {
    if (char current = line[cursor_]; current == '_' || std::isalpha(current)) {
        std::string buffer;
        while (current == '_' || std::isalnum(current)) {
            buffer.push_back(current);
            current = line[++cursor_];
        }

        if (not MatchedKeyword(buffer)) {
            AddToken(TokenType::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
HEX_NODISCARD bool Lexer::LexedString(const std::string_view line) {
    std::string buffer;

    char current_char = line[cursor_];
    buffer.push_back(current_char);

    TokenType literal_type;
    switch (current_char) {
    case '\"':
        literal_type = TokenType::Lit_String;
        break;
    case '\'':
        literal_type = TokenType::Lit_Char;
        break;
    default:
        Log(LogLevel::Error, "Improper call to LexedString");
        AddToken(TokenType::Unknown, std::move(buffer));
        return false;
    }

    while (true) {
        if (static_cast<usize>(++cursor_) >= line.size()) {
            // next token should always be a newline or string literal
            Log(LogLevel::Warn, "Unexpected EOF while lexing string literal");
            AddToken(TokenType::Unknown, std::move(buffer));
            AddEOF();
            return false;
        }

        current_char = line[cursor_];  // need to update

        if (current_char == '\n' ||
            (current_char == '\\' && line[cursor_ + 1] == 'n')) {
            // strings must close on the line they're started
            return false;
        }

        buffer.push_back(current_char);

        if (current_char == '\'' || current_char == '\"') {
            ++cursor_;
            break;
        }
    }

    AddToken(literal_type, std::move(buffer));
    return true;
}

HEX_NODISCARD bool Lexer::LexedNumber(const std::string_view line) {
    const bool is_negative = line[cursor_] == '-' && std::isdigit(line[cursor_ + 1]);

    if (not is_negative && not std::isdigit(line[cursor_])) {
        return false;
    }

    // Confirmed digit, consume char
    std::string buffer;
    buffer.push_back(line[cursor_++]);

    // INT = ^[-?0-9]+
    const auto eat_digits = [&] {
        while (std::isdigit(line[cursor_])) {
            buffer.push_back(line[cursor_++]);
        }
    };

    eat_digits();

    // FLOAT = INT.[0-9]+
    // if we encounter a dot, it can't be an int
    if (line[cursor_] != '.') {
        AddToken(TokenType::Lit_Int, std::move(buffer));
        return true;
    }

    // have to eat the dot first
    buffer.push_back(line[cursor_++]);
    eat_digits();

    AddToken(TokenType::Lit_Float, std::move(buffer));
    return true;
}

HEX_NODISCARD bool Lexer::LexedOperator(const std::string_view line) {
    const auto current = line[cursor_];
    const auto next    = line[cursor_ + 1];
    TokenType  token_type;

    switch (current) {
        using enum TokenType;

    case '=':
        if (next == '=') {
            token_type = Op_Equality;  // ==
            break;
        }
        token_type = Op_Assign;
        break;
    case '+':
        token_type = Op_Plus;
        break;
    case '-':
        if (next == '>') {
            token_type = Op_Arrow;  // ->
            break;
        }
        token_type = Op_Minus;
        break;
    case '*':
        token_type = Op_Star;
        break;
    case '/':
        token_type = Op_FwdSlash;
        break;
    case ':':
        if (next == ':') {
            token_type = Op_ModuleElementAccess;  // ::
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
        if (next == '=') {
            token_type = Op_NotEqual;  // !=
            break;
        }
        token_type = Op_LogicalNot;
        break;
    case '<':
        if (next == '=') {
            token_type = Op_LessEqual;  // <=
            break;
        }
        token_type = Op_LessThan;
        break;
    case '>':
        if (next == '=') {
            token_type = Op_GreaterEqual;  // >=
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
        return LexedString(line);

    default:
        return false;
    }

    std::string buffer(1, current);

    switch (token_type) {
        using enum TokenType;

    case Op_ModuleElementAccess:
    case Op_Equality:
    case Op_NotEqual:
    case Op_LessEqual:
    case Op_GreaterEqual:
        [[fallthrough]];
    case Op_Arrow:
        buffer.push_back(next);
        ++cursor_;
        break;

    default:
        break;
    }

    AddToken(token_type, std::move(buffer));
    ++cursor_;

    return true;
}

void Lexer::LexUnknown(const std::string_view line) {
    std::string buffer;
    while (not IsWhitespace(line[cursor_])) {
        buffer.push_back(line[cursor_++]);
    }

    if (not buffer.empty()) {
        AddToken(TokenType::Unknown, std::move(buffer));
    }
}

// we take a string ref because we have a string that
// we'd otherwise need to construct from a stringview anyway
HEX_NODISCARD bool Lexer::MatchedKeyword(std::string& identifier_buffer) {
    using KeywordMap = std::unordered_map<std::string, TokenType>;

    using enum TokenType;
    static const KeywordMap keyword_map = {
        {"i8", KW_i8},           {"i16", KW_i16},       {"i32", KW_i32},
        {"i64", KW_i64},         {"i128", KW_i128},

        {"u8", KW_u8},           {"u16", KW_u16},       {"u32", KW_u32},
        {"u64", KW_u64},         {"u128", KW_u128},

        {"f32", KW_f32},         {"f64", KW_f64},

        {"byte", KW_byte},       {"char", KW_char},     {"string", KW_string},

        {"bool", KW_bool},       {"null", Lit_null},

        {"data", KW_data},       {"fn", KW_fn},         {"mut", KW_mut},
        {"raw", KW_raw},         {"const", KW_const},   {"override", KW_override},

        {"pack", KW_pack},       {"struct", KW_struct}, {"enum", KW_enum},
        {"generic", KW_generic},

        {"module", KW_module},   {"public", KW_public}, {"private", KW_private},
        {"import", KW_import},   {"as", KW_as},

        {"return", KW_return},   {"true", Lit_true},     {"false", Lit_false},
        {"if", KW_if},           {"else", KW_else},     {"match", KW_match},

        {"loop", KW_loop},       {"while", KW_while},   {"for", KW_for},
        {"break", KW_break},     {"skip", KW_skip},

        {"and", Op_LogicalAnd},  {"or", Op_LogicalOr},  {"not", Op_LogicalNot},
    };

    if (const auto keyword = keyword_map.find(identifier_buffer);
        keyword != keyword_map.end()) {
        AddToken(keyword->second, std::move(identifier_buffer));
        return true;
    }

    return false;
}

HEX_NODISCARD bool Lexer::IsWhitespace(const char c) const {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

HEX_NODISCARD bool Lexer::IsComment(const char c) const {
    return c == '#';
}

void Lexer::AddToken(TokenType type, std::string& text) {
    const i64 column_pos = cursor_ + 1 - static_cast<i64>(text.length());
    token_stream_.emplace_back(
        type,
        text,
        TextPosition {
            .line   = line_number_,
            .column = column_pos,  // column counts from 1
        }
    );
}

void Lexer::AddToken(TokenType type, std::string&& text) {
    const i64 column_pos = cursor_ + 1 - static_cast<i64>(text.length());
    token_stream_.emplace_back(
        type,
        std::move(text),
        TextPosition {
            .line = line_number_,
            .column = column_pos
        }
    );
}

void Lexer::AddEOF() {
    token_stream_.emplace_back(
        TOKEN_EOF.type,
        TOKEN_EOF.text,
        TextPosition {
            .line   = line_number_,
            .column = cursor_ +
                      2  // display EOF tokens as being out of bounds of file contents
        }
    );
}
}  // namespace hex