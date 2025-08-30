#include <sigil/ast/keywords.hpp>
#include <sigil/ast/lexer.hpp>
#include <sigil/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>

namespace sigil {
using namespace mana::literals;

Lexer::Lexer()
    : cursor {0}
    , line_start(0)
    , line_number {0} {}

bool Lexer::IsTerminator() const {
    return source[cursor] == '\n';  // add semicolons in the future?
}

void Lexer::TokenizeLine() {
    line_start = cursor;

    while (cursor < source.length() && not IsTerminator()) {
        if (IsLineComment(source[cursor])) {
            do {
                ++cursor;
            } while (not IsTerminator());
            break;
        }

        if (IsWhitespace(source[cursor])) {
            ++cursor;
            continue;
        }

        if (LexedNumber()) {
            continue;
        }

        if (LexedIdentifier()) {
            continue;
        }

        if (LexedOperator()) {
            continue;
        }

        LexUnknown();
    }

    AddToken(TokenType::Terminator, "\n");

    ++line_number;
    ++cursor;
}

bool Lexer::Tokenize(const std::filesystem::path& file_path) {
    std::ifstream source_file(file_path);
    if (not source_file.is_open()) {
        Log->error("Failed to open file at '{}'", file_path.string());
        return false;
    }

    Reset();

    const auto file_size = std::filesystem::file_size(file_path);
    source.resize(file_size);
    source_file.read(source.data(), file_size);
    source_file.close();

    token_stream.emplace_back(TokenType::_artifact_,
                              file_path.filename().replace_extension("").string(),
                              TextPosition {
                                  .line   = -1,
                                  .column = -1,
                              });

    // lines count from 1
    line_number = 1;
    while (cursor < file_size) {
        TokenizeLine();
    }

    AddEOF();
    return true;
}

void Lexer::PrintTokens() const {
    if (token_stream.empty()) {
        Log->error("Lexer token print requested, but token stream was empty.");
        return;
    }

    Log->debug("--- Printing Token Stream ---\n");

    for (const auto& [type, contents, position] : token_stream) {
        if (type == TokenType::Terminator) {
            Log->info("[L: {} | C: {}] {}: \\n",
                      position.line,
                      position.column,
                      magic_enum::enum_name(type));
            continue;
        }
        Log->info("[L: {} | C: {}] {}: \\n",
                  position.line,
                  position.column,
                  magic_enum::enum_name(type),
                  contents);
    }
    Log->debug("End of token stream.\n");
}

void Lexer::Reset() {
    token_stream.clear();
    source.clear();

    cursor      = 0;
    line_number = 0;
    line_start  = 0;
}

TokenStream&& Lexer::RelinquishTokens() {
    return std::move(token_stream);
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
bool Lexer::LexedIdentifier() {
    if (char current = source[cursor]; current == '_' || std::isalpha(current)) {
        std::string buffer;
        while (current == '_' || std::isalnum(current)) {
            buffer.push_back(current);
            current = source[++cursor];
        }

        if (not MatchedKeyword(buffer)) {
            AddToken(TokenType::Identifier, std::move(buffer));
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
bool Lexer::LexedString() {
    std::string buffer;

    char current_char = source[cursor];
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
        Log->error("Improper call to LexedString");
        AddToken(TokenType::Unknown, std::move(buffer));
        return false;
    }

    while (true) {
        if (static_cast<usize>(++cursor) >= source.size()) {
            // next token should always be a newline or string literal
            Log->warn("Unexpected EOF while lexing string literal");
            AddToken(TokenType::Unknown, std::move(buffer));
            AddEOF();
            return false;
        }

        current_char = source[cursor];  // need to update

        if (current_char == '\n' || (current_char == '\\' && source[cursor + 1] == 'n')) {
            // strings must close on the line they're started
            return false;
        }

        buffer.push_back(current_char);

        if (current_char == '\'' || current_char == '\"') {
            ++cursor;
            break;
        }
    }

    AddToken(literal_type, std::move(buffer));
    return true;
}

bool Lexer::LexedNumber() {
    if (not std::isdigit(source[cursor])) {
        return false;
    }

    std::string buffer;

    // INT = ^[-?0-9]+
    const auto eat_digits = [&] {
        while (std::isdigit(source[cursor])) {
            buffer.push_back(source[cursor++]);
        }
    };

    eat_digits();

    // FLOAT = INT.[0-9]+
    // if we encounter a dot, it can't be an int
    if (source[cursor] != '.') {
        AddToken(TokenType::Lit_Int, std::move(buffer));
        return true;
    }

    // have to eat the dot first
    buffer.push_back(source[cursor++]);
    eat_digits();

    AddToken(TokenType::Lit_Float, std::move(buffer));
    return true;
}

bool Lexer::LexedOperator() {
    const auto current = source[cursor];
    const auto next    = source[cursor + 1];
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
        token_type = Op_Asterisk;
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
        return LexedString();

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
    case Op_Arrow:
        buffer.push_back(next);
        ++cursor;
        break;

    default:
        break;
    }

    ++cursor;
    AddToken(token_type, std::move(buffer));

    return true;
}

void Lexer::LexUnknown() {
    std::string buffer;
    while (not IsWhitespace(source[cursor])) {
        buffer.push_back(source[cursor++]);
    }

    if (not buffer.empty()) {
        AddToken(TokenType::Unknown, std::move(buffer));
    }
}

// we take a string ref because
// we have a string that we'd otherwise need to construct from a string_view anyway
bool Lexer::MatchedKeyword(std::string& identifier_buffer) {
    if (const auto keyword = keyword_map.find(identifier_buffer);
        keyword != keyword_map.end()) {
        AddToken(keyword->second, std::move(identifier_buffer));
        return true;
    }

    return false;
}

bool Lexer::IsWhitespace(const char c) const {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

bool Lexer::IsLineComment(const char c) const {
    return c == '#';
}

i64 Lexer::GetTokenColumnIndex(const std::size_t token_length) {
    // column counts from 1
    return 1 + (cursor - line_start) - token_length;
}

void Lexer::AddToken(TokenType type, char c) {}

void Lexer::AddToken(TokenType type, std::string& text) {
    token_stream.emplace_back(type,
                              text,
                              TextPosition {
                                  .line   = line_number,
                                  .column = GetTokenColumnIndex(text.length()),
                              });
}

void Lexer::AddToken(TokenType type, std::string&& text) {
    token_stream.emplace_back(type,
                              std::move(text),
                              TextPosition {.line = line_number,
                                            .column = GetTokenColumnIndex(text.length())});
}

void Lexer::AddEOF() {
    token_stream.emplace_back(TOKEN_EOF.type,
                              TOKEN_EOF.text,
                              TextPosition {
                                  .line   = line_number + 1,
                                  .column = -1,
                                  // display EOF tokens as being out of bounds of file contents
                              });
}
}  // namespace sigil