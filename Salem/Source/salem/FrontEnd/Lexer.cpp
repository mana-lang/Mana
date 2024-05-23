#include <Salem/FrontEnd/Lexer.hpp>
#include <Salem/Core/Logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <fstream>
#include <array>

namespace salem {

// all contextual symbols (e.g. '-' unary vs. subtraction) should be handled before
// the operators method, which crystallizes them into regular operators
// where the hell is the obj file for this
Lexer::Lexer()
    : cursor_(0)
    , line_number_()
{}

void Lexer::AddToken(const Token::Type type, const std::string&& contents) {
    token_stream_.emplace_back(Token(type, std::forward<const std::string>(contents),
        {line_number_, cursor_ + 1}));
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
        ++line_number_;

        if (current_line.empty()) {
            continue;
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

        if (not LexKeywords(buffer)) {
            AddToken(Token::Type::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

bool Lexer::LexKeywords(std::string& ident_buffer) {
    static const std::array keyword_tokens = {
        Token(Token::Type::KW_i8, "i8"),
        Token(Token::Type::KW_i16, "i16"),
        Token(Token::Type::KW_i32, "i32"),
        Token(Token::Type::KW_i64, "i64"),
        Token(Token::Type::KW_i128, "i128"),

        Token(Token::Type::KW_u8, "u8"),
        Token(Token::Type::KW_u16, "u16"),
        Token(Token::Type::KW_u32, "u32"),
        Token(Token::Type::KW_u64, "u64"),
        Token(Token::Type::KW_u128, "u128"),

        Token(Token::Type::KW_f32, "f32"),
        Token(Token::Type::KW_f64, "f64"),
    };

    for (const auto& [token_type, keyword_name, _] : keyword_tokens) {
        if (ident_buffer == keyword_name) {
            AddToken(token_type, std::move(ident_buffer));
            return true;
        }
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

bool Lexer::IsWhitespace(const char c) {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

bool Lexer::IsComment(const char c) {
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
    Token::Type token_type;

    switch (current_char) {
        using enum Token::Type;
    case '=':
        token_type = Op_Assign;
        break;
    case '+':
        token_type = Op_Add;
        break;
    case '-':
        token_type = Op_Sub;
        break;
    case '*':
        token_type = Op_Mul;
        break;
    case '/':
        token_type = Op_Div;
        break;
    case ':':
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
    default:
        return false;
    }

    AddToken(token_type, std::string(1, current_char));
    ++cursor_;
    return true;
}

}  // namespace salem