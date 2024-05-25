#include <Salem/FrontEnd/Lexer.hpp>
#include <Salem/Core/Logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <fstream>
#include <unordered_map>

namespace salem {

// all contextual symbols (e.g. '-' unary vs. subtraction) should be handled before
// the operators method, which crystallizes them into regular operators
// where the hell is the obj file for this
Lexer::Lexer()
    : cursor_(0)
    , line_number_()
{}

void Lexer::TokenizeLine(const std::string_view current_line) {
    ++line_number_;

    if (current_line.empty()) {
        return;
    }
    cursor_ = 0;

    const auto line_size = current_line.size();
    while (cursor_ < line_size) {
        if (IsComment(current_line[cursor_])) {
            break;
        }

        if (IsWhitespace(current_line[cursor_])) {
            ++cursor_;
            continue;
        }

        if (LexNumbers(current_line)) {
            continue;
        }

        if (LexIdentifiers(current_line)) {
            continue;
        }

        if (LexOperators(current_line)) {
            continue;
        }

        LexUnknown(current_line);
    }

    AddToken(Token::Type::Newline, "\n");
}

void Lexer::AddToken(const Token::Type type, std::string&& contents) {
    token_stream_.emplace_back(
            type,
            std::move(contents),
            TextPosition{line_number_, cursor_ + 1}
    );
}

bool Lexer::TokenizeFile(const std::filesystem::path& path_to_file) {
    token_stream_.clear();

    std::ifstream file(path_to_file);
    if (not file.is_open()) {
        Log(LogLevel::Error, "Failed to open file at '{}'", path_to_file.string());
        return false;
    }

    std::string current_line;
    while (std::getline(file, current_line)) {
        current_line.push_back('\n'); // reinsert delimiter for seek operations
        TokenizeLine(current_line);
    }

    AddToken(Token::Type::Eof, "EOF");
    line_number_ = 0;
    cursor_      = 0;
    return true;
}

void Lexer::PrintTokens() const {
    if (token_stream_.empty()) {
        Log(LogLevel::Error, "Lexer token print requested, but token stream was empty.");
        return;
    }


    Log(LogLevel::Debug, "--- Printing Token Stream ---\n");

    for (const auto& t : token_stream_) {
        if (t.type == Token::Type::Newline) {
            Log(LogLevel::Info, "[L: {} | C: {}] {}: \\n", t.position.line, t.position.column, magic_enum::enum_name(t.type));
            continue;
        }
        Log(LogLevel::Info, "[L: {} | C: {}] {}: {}", t.position.line, t.position.column, magic_enum::enum_name(t.type), t.contents);
    }
}

void Lexer::Clear() {
    token_stream_.clear();
}

auto Lexer::RelinquishTokens() -> std::vector<Token>&& {
    return std::move(token_stream_);
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
bool Lexer::LexIdentifiers(const std::string_view current_line) {
    if (current_line[cursor_] == '_' || std::isalpha(current_line[cursor_])) {
        std::string buffer;
        while (current_line[cursor_] == '_' || std::isalnum(current_line[cursor_])) {
            buffer.push_back(current_line[cursor_++]);
        }

        if (not MatchKeyword(buffer)) {
            AddToken(Token::Type::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

bool Lexer::MatchKeyword(std::string& ident_buffer) {
    using TokenMap = std::unordered_map<std::string, Token::Type>;
    using enum Token::Type;
    static const TokenMap keyword_map = {
        {"i8",   KW_i8},
        {"i16",  KW_i16},
        {"i32",  KW_i32},
        {"i64",  KW_i64},
        {"i128", KW_i128},

        {"u8",   KW_u8},
        {"u16",  KW_u16},
        {"u32",  KW_u32},
        {"u64",  KW_u64},
        {"u128", KW_u128},

        {"f32", KW_f32},
        {"f64", KW_f64},

        {"byte",   KW_byte},
        {"char",   KW_char},
        {"string", KW_string},
        {"bool",   KW_bool},
        {"void",   KW_void},

        {"data",     KW_data},
        {"fn",       KW_fn},
        {"mut",      KW_mut},
        {"raw",      KW_raw},
        {"const",    KW_const},
        {"override", KW_override},

        {"pack",    KW_pack},
        {"struct",  KW_struct},
        {"enum",    KW_enum},
        {"generic", KW_generic},

        {"module",  KW_module},
        {"public",  KW_public},
        {"private", KW_private},
        {"import",  KW_import},
        {"as",      KW_as},

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
        {"or",  Op_LogicalOr},
        {"not", Op_LogicalNot},
    };

    if (const auto keyword = keyword_map.find(ident_buffer);
            keyword != keyword_map.end()
            ) {
        AddToken(keyword->second, std::move(ident_buffer));
        return true;
    }
    return false;
}

void Lexer::LexUnknown(const std::string_view current_line) {
    std::string buffer;
    while (!IsWhitespace(current_line[cursor_])) {
        buffer.push_back(current_line[cursor_++]);
    }

    if (not buffer.empty()) {
        AddToken(Token::Type::Unknown, std::move(buffer));
    }
}

bool Lexer::IsWhitespace(const char c) const {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

bool Lexer::IsComment(const char c) const {
    return c == '#';
}

// NUMBER = INT | FLOAT
bool Lexer::LexNumbers(const std::string_view current_line) {
    const auto current_char = current_line[cursor_];

    const bool is_negative_digit = current_char == '-' && std::isdigit(current_line[cursor_ + 1]);
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
        AddToken(Token::Type::Int, std::move(buffer));
        return true;
    }

    // eat dot before progressing
    buffer.push_back(current_line[cursor_++]);

    eat_digits();

    AddToken(Token::Type::Float, std::move(buffer));
    return true;
}

bool Lexer::LexOperators(const std::string_view current_line) {
    const auto current_char = current_line[cursor_];
    const auto next_char = current_line[cursor_ + 1];
    Token::Type token_type;

    switch (current_char) {
        using enum Token::Type;
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
        token_type = Sym_StringLiteral;
        break;
    case '\'':
        token_type = Sym_CharLiteral;
        break;

    default:
        return false;
    }

    std::string buffer(1, current_char);

    switch (token_type) {
        using enum Token::Type;

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

    AddToken(token_type, std::move(buffer));
    ++cursor_;

    return true;
}

}  // namespace salem