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

thread_local GlobalSourceFile Lexer::Source;

Lexer::Lexer()
    : cursor{0}
    , line_start(0)
    , line_number{0} {}

bool Lexer::IsTerminator() const {
    return Source[cursor] == '\n'; // add semicolons in the future?
}

void Lexer::TokenizeLine() {
    line_start = cursor;

    while (cursor < Source.Size() && not IsTerminator()) {
        if (IsLineComment(Source[cursor])) {
            do { ++cursor; }
            while (not IsTerminator());

            break;
        }

        if (IsWhitespace(Source[cursor])) {
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

    ++cursor;
    AddToken(TokenType::Terminator, 1);

    ++line_number;
}

bool Lexer::Tokenize(const std::filesystem::path& file_path) {
    Reset();
    Source.Load(file_path);

    // lines count from 1
    line_number = 1;
    while (cursor < Source.Size()) {
        TokenizeLine();
    }

    AddEOF();
    return true;
}

void Lexer::PrintTokens() const {
    if (tokens.empty()) {
        Log->error("Lexer token print requested, but token stream was empty.");
        return;
    }

    Log->debug("--- Printing Token Stream ---\n");

    for (const auto& [line, offset, column, length, type] : tokens) {
        if (type == TokenType::Eof) {
            Log->info("[{}:{}] {}: EOF",
                      line,
                      column,
                      magic_enum::enum_name(type));
            continue;
        }
        if (type == TokenType::Terminator) {
            Log->info("[{}:{}] {}: \\n",
                      line,
                      column,
                      magic_enum::enum_name(type));
            continue;
        }
        Log->info("[{}:{}] {}: {}",
                  line,
                  column,
                  magic_enum::enum_name(type),
                  Source.Slice(offset, length));
    }
    Log->debug("End of token stream.\n");
}

void Lexer::Reset() {
    tokens.clear();
    Source.Reset();

    cursor      = 0;
    line_number = 0;
    line_start  = 0;
}

std::vector<Token>&& Lexer::RelinquishTokens() {
    return std::move(tokens);
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
bool Lexer::LexedIdentifier() {
    if (char current = Source[cursor];
        current == '_' || std::isalpha(current)) {
        std::string buffer;
        while (current == '_' || std::isalnum(current)) {
            buffer.push_back(current);
            current = Source[++cursor];
        }

        if (not MatchedKeyword(buffer)) {
            AddToken(TokenType::Identifier, static_cast<u16>(buffer.length()));
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
bool Lexer::LexedString() {
    // start with current char, so length is 1
    u16       length = 1;
    TokenType literal_type;

    char current_char = Source[cursor];
    switch (current_char) {
    case '\"':
        literal_type = TokenType::Lit_String;
        break;
    case '\'':
        literal_type = TokenType::Lit_Char;
        break;
    default:
        Log->error("Improper call to LexedString");
        AddToken(TokenType::Unknown, length);
        return false;
    }

    while (true) {
        if (static_cast<usize>(++cursor) >= Source.Size()) {
            Log->warn("Unexpected EOF while lexing string literal");
            AddToken(TokenType::Unknown, length);
            AddEOF();
            return false;
        }

        current_char = Source[cursor];

        // strings must close on the line they're started
        if (current_char == '\n' || (current_char == '\\' && Source[cursor + 1] == 'n')) {
            return false;
        }

        ++length;

        // end of string
        if (current_char == '\'' || current_char == '\"') {
            ++cursor;
            break;
        }
    }

    AddToken(literal_type, length);
    return true;
}

bool Lexer::LexedNumber() {
    if (not std::isdigit(Source[cursor])) {
        return false;
    }

    u16 length = 0;

    // INT = ^[-?0-9]+
    const auto eat_digits = [&] {
        while (std::isdigit(Source[cursor])) {
            ++length;
            ++cursor;
        }
    };

    eat_digits();

    // FLOAT = INT.[0-9]+
    // if we encounter a dot, it can't be an int
    if (Source[cursor] != '.') {
        AddToken(TokenType::Lit_Int, length);
        return true;
    }

    // have to eat the dot first
    ++length;
    ++cursor;
    eat_digits();

    AddToken(TokenType::Lit_Float, length);
    return true;
}

bool Lexer::LexedOperator() {
    const auto current = Source[cursor];
    const auto next    = Source[cursor + 1];
    TokenType  token_type;

    switch (current) {
        using enum TokenType;

    case '=':
        if (next == '=') {
            token_type = Op_Equality; // ==
            break;
        }
        token_type = Op_Assign;
        break;
    case '+':
        token_type = Op_Plus;
        break;
    case '-':
        if (next == '>') {
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
        if (next == ':') {
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
        if (next == '=') {
            token_type = Op_NotEqual; // !=
            break;
        }
        token_type = Op_LogicalNot;
        break;
    case '<':
        if (next == '=') {
            token_type = Op_LessEqual; // <=
            break;
        }
        token_type = Op_LessThan;
        break;
    case '>':
        if (next == '=') {
            token_type = Op_GreaterEqual; // >=
            break;
        }
        token_type = Op_GreaterThan;
        break;
    case '&':
        token_type = Op_Assign_Ref;
        break;
    case '~':
        token_type = Op_Assign_Move;
        break;
    case '$':
        token_type = Op_Assign_Copy;
        break;
    case '\"':
    case '\'':
        return LexedString();

    default:
        return false;
    }

    u16 token_length = 1;

    switch (token_type) {
        using enum TokenType;

    case Op_ModuleElementAccess:
    case Op_Equality:
    case Op_NotEqual:
    case Op_LessEqual:
    case Op_GreaterEqual:
    case Op_Arrow:
        ++token_length;
        ++cursor;
        break;

    default:
        break;
    }

    ++cursor;
    AddToken(token_type, token_length);

    return true;
}

void Lexer::LexUnknown() {
    u16 length = 0;
    while (not IsWhitespace(Source[cursor])) {
        ++cursor;
        ++length;
    }

    if (length > 0) {
        AddToken(TokenType::Unknown, length);
    }
}

bool Lexer::MatchedKeyword(const std::string_view identifier) {
    if (const auto keyword = keyword_map.find(identifier.data());
        keyword != keyword_map.end()) {
        AddToken(keyword->second, identifier.length());
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

u16 Lexer::GetTokenColumnIndex(const u16 token_length) const {
    // column counts from 1
    return 1 + (cursor - line_start) - token_length;
}

void Lexer::AddToken(const TokenType type, const u16 length) {
    tokens.emplace_back(Token{
        .line = line_number,
        .offset = cursor - length,
        .column = GetTokenColumnIndex(length),
        .length = length,
        .type = type,
    });
}

void Lexer::AddEOF() {
    // line number will be out of bounds once this gets called,
    // which is what we want for EOF
    tokens.emplace_back(Token{
        .line = line_number,
        .offset = cursor,
        .column = 0,
        .length = 0,
        .type = TokenType::Eof,
    });
}
} // namespace sigil
