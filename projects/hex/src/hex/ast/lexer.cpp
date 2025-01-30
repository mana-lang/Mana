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
HEX_NODISCARD bool Lexer::LexedIdentifier(std::string_view line) {
    if (const char current = line[cursor_]; current == '_' || std::isalpha(current)) {
        std::string buffer;
        while (current == '_' || std::isalnum(current)) {
            buffer.push_back(current);
            ++cursor_;
        }

        if (not MatchedKeyword(buffer)) {
            AddToken(TokenType::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
HEX_NODISCARD bool Lexer::LexedString(std::string_view line) {
    std::string buffer;
    char        current = line[cursor_];
    buffer.push_back(current);

    TokenType literal_type;
    switch (current) {
    case '\"':
        literal_type = TokenType::Lit_String;
        break;
    case '\'':
        literal_type = TokenType::Lit_String;
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

        current = line[cursor_];  // need to update

        if (current == '\n' || (current == '\\' && line[cursor_ + 1] == 'n')) {
            // strings must close on the line they're started
            return false;
        }

        buffer.push_back(current);

        if (current == '\'' || current == '\"') {
            ++cursor_;
            break;
        }
    }

    AddToken(literal_type, std::move(buffer));
    return true;
}

HEX_NODISCARD bool Lexer::LexedNumber(std::string_view line) {}

HEX_NODISCARD bool Lexer::LexedOperator(std::string_view line) {}

void Lexer::LexUnknown(std::string_view line) {}

HEX_NODISCARD bool Lexer::MatchedKeyword(std::string& identifier_buffer) {}

HEX_NODISCARD bool Lexer::IsWhitespace(char c) const {}

HEX_NODISCARD bool Lexer::IsComment(char c) const {}

void Lexer::AddToken(TokenType type, std::string& text) {
    token_stream_.emplace_back(
        type,
        text,
        TextPosition {
            .line   = line_number_,
            .column = cursor_ + 1  // column counts from 1
        }
    );
}

void Lexer::AddToken(TokenType type, std::string&& text) {
    token_stream_
        .emplace_back(type, std::move(text), TextPosition {.line = line_number_, .column = cursor_ + 1});
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