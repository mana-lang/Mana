#include <sigil/ast/keywords.hpp>
#include <sigil/ast/lexer.hpp>
#include <sigil/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/sinks/basic_file_sink.h>

#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>

namespace sigil {
using namespace mana::literals;

thread_local GlobalSourceFile Lexer::Source;

Lexer::Lexer()
    : cursor {0},
      line_start(0),
      line_number {0} {}

bool Lexer::IsNewline() const {
    return Source[cursor] == '\n';
}

void Lexer::TokenizeLine() {
    line_start = cursor;

    while (cursor < Source.Size() && not IsNewline()) {
        if (IsLineComment()) {
            do { ++cursor; }
            while (not IsNewline());

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

void Lexer::Reset() {
    tokens.clear();
    Source.Reset();

    cursor      = 0;
    line_number = 0;
    line_start  = 0;
}

usize Lexer::TokenCount() const {
    return tokens.size();
}

std::vector<Token>&& Lexer::RelinquishTokens() {
    return std::move(tokens);
}

const std::vector<Token>& Lexer::Tokens() const {
    return tokens;
}

// ID = ^[a-zA-Z_][a-zA-Z0-9_]+
bool Lexer::LexedIdentifier() {
    if (char current = Source[cursor];
        current == '_' || std::isalpha(current)) {
        const u16 start = cursor;
        u16 length      = 0;

        while (current == '_' || std::isalnum(current)) {
            ++length;
            current = Source[++cursor];
        }

        if (not MatchedKeyword(Source.Slice(start, length))) {
            AddToken(TokenType::Identifier, length);
        }
        return true;
    }

    return false;
}

// only to be entered when current char is " or '
bool Lexer::LexedString() {
    // start with current char, so length is 1
    u16 length = 1;
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

    const auto starting_char = current_char;
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
            Log->error("Unexpected end of string literal");
            AddToken(TokenType::Unknown, length);
            return false;
        }

        ++length;

        // end of string
        if (current_char == starting_char) {
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
    if (Source[cursor] != '.' // two dots could be a range operator, so we know it's an int
        || Source[cursor + 1] == '.') {
        AddToken(TokenType::Lit_Int, length);
        return true;
    }

    // have to eat the dot first
    ++length;
    ++cursor;

    if (not std::isdigit(Source[cursor])) {
        Log->error("Incomplete float literal.");
        AddToken(TokenType::Unknown, length);
        return false;
    }

    eat_digits();

    AddToken(TokenType::Lit_Float, length);
    return true;
}

bool Lexer::LexedOperator() {
    const auto current = Source[cursor];
    const auto next    = Source[cursor + 1];
    TokenType token_type;
    //
    switch (current) {
        using enum TokenType;

    case '=':
        if (next == '=') {
            token_type = Op_Equality; // ==
            break;
        }
        if (next == '>') {
            token_type = Op_Binding; // =>
            break;
        }
        token_type = Op_Assign;
        break;
    case '+':
        if (next == '=') {
            token_type = Op_AddAssign;
            break;
        }
        token_type = Op_Plus;
        break;
    case '-':
        if (next == '>') {
            token_type = Op_ReturnType; // ->
            break;
        }
        if (next == '=') {
            token_type = Op_SubAssign;
            break;
        }
        token_type = Op_Minus;
        break;
    case '*':
        if (next == '=') {
            token_type = Op_MulAssign;
            break;
        }
        token_type = Op_Asterisk;
        break;
    case '/':
        if (next == '=') {
            token_type = Op_DivAssign;
            break;
        }
        token_type = Op_FwdSlash;
        break;
    case '%':
        if (next == '=') {
            token_type = Op_ModAssign;
            break;
        }
        token_type = Op_Modulo;
        break;
    case ':':
        if (next == ':') {
            token_type = Op_ScopeResolution; // ::
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
        if (next == '.') {
            token_type = Op_Range;
            break;
        }
        token_type = Op_Access;
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
    case '|':
        if (next == '|') {
            token_type = Op_LogicalOr; // ||
            break;
        }
        token_type = Op_MultiMatch;
        break;
    case '&':
        if (next == '&') {
            token_type = Op_LogicalAnd; // &&
            break;
        }
        token_type = Op_Ref;
        break;
    case '~':
        token_type = Op_Move;
        break;
    case '$':
        token_type = Op_Copy;
        break;
    case ';':
        token_type = Terminator;
        break;
    case '@':
        token_type = Op_Attribute;
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

    case Op_ScopeResolution:
    case Op_Equality:
    case Op_NotEqual:
    case Op_LessEqual:
    case Op_GreaterEqual:
    case Op_ReturnType:
    case Op_LogicalAnd:
    case Op_LogicalOr:
    case Op_Binding:
    case Op_Range:
    case Op_AddAssign:
    case Op_SubAssign:
    case Op_MulAssign:
    case Op_DivAssign:
    case Op_ModAssign:
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
    if (const auto keyword = KEYWORDS.find(identifier);
        keyword != KEYWORDS.end()) {
        AddToken(keyword->second, identifier.length());
        return true;
    }

    return false;
}

bool Lexer::IsWhitespace(const char c) const {
    return c == ' ' || c == '\0' || c == '\n' || c == '\r' || c == '\t';
}

bool Lexer::IsLineComment() const {
    return Source[cursor] == '/' && Source[cursor + 1] == '/';
}

u16 Lexer::GetTokenColumnIndex(const u16 token_length) const {
    // column counts from 1
    return 1 + (cursor - line_start) - token_length;
}

void Lexer::AddToken(const TokenType type, const u16 length) {
    tokens.emplace_back(Token {
            .line   = line_number,
            .offset = cursor - length,
            .column = GetTokenColumnIndex(length),
            .length = length,
            .type   = type,
        }
    );
}

void Lexer::AddEOF() {
    // line number will be out of bounds once this gets called,
    // which is what we want for EOF
    tokens.emplace_back(Token {
            .line   = line_number,
            .offset = cursor,
            .column = 0,
            .length = 0,
            .type   = TokenType::Eof,
        }
    );
}

void PrintTokens(const std::vector<Token>& tokens, const PrintingMode mode, const PrintingPolicy policy) {
    if (tokens.empty()) {
        Log->error("Lexer token print requested, but token stream was empty.");
        return;
    }

    if (mode == PrintingMode::Emit) {
        Log->sinks().push_back(
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                std::string(Source().Name()) + ".tks",
                true
            )
        );
        Log->set_pattern("%v");
    }

    Log->debug("Token Stream for '{}'\n", Source().Name());

    constexpr auto align_pos   = 4;
    constexpr auto align_token = 15;
    for (const auto& [line, offset, column, length, type] : tokens) {
        const auto token_type_name = magic_enum::enum_name(type);

        // formats as "  line:column  " where the colon is always in the same spot.
        const auto pos_base = fmt::format("{:>{}}:{:<{}}",
                                          line,
                                          align_pos,
                                          column,
                                          align_pos
        );

        // handle eof separately
        if (type == TokenType::Eof) {
            Log->debug("{} => EOF", pos_base);
            continue;
        }

        if (type == TokenType::Terminator && policy == PrintingPolicy::SkipTerminators) {
            continue;
        }

        const auto value = type == TokenType::Terminator ? "\\n" : Source().Slice(offset, length);

        Log->debug("{} => {:<{}} -->  {}",
                   pos_base,
                   token_type_name,
                   align_token,
                   value
        );
    }

    Log->debug("");
    Log->debug("End of token stream.\n");

    if (mode == PrintingMode::Emit) {
        Log->sinks().pop_back();
        Log->set_pattern(mana::GlobalLoggerSink().DefaultPattern);
    }
}
} // namespace sigil
