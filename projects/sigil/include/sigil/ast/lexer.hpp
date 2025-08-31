#pragma once

#include <sigil/ast/token.hpp>
#include <sigil/ast/source-file.hpp>

#include <mana/literals.hpp>

#include <filesystem>
#include <string>

namespace sigil {
namespace ml = mana::literals;

class Lexer {
    ml::i32 cursor;

    ml::i32 line_start;
    ml::i32 line_number;

    TokenStream tokens;

public:
    static thread_local GlobalSourceFile Source;

    Lexer();

    bool Tokenize(const std::filesystem::path& file_path);
    void PrintTokens() const;
    void Reset();

    SIGIL_NODISCARD TokenStream&& RelinquishTokens();

private:
    void TokenizeLine();

    SIGIL_NODISCARD ml::u16 GetTokenColumnIndex(ml::u16 token_length) const;

    void AddToken(TokenType type, ml::u16 length);

    SIGIL_NODISCARD bool LexedIdentifier();
    SIGIL_NODISCARD bool LexedString();
    SIGIL_NODISCARD bool LexedNumber();
    SIGIL_NODISCARD bool LexedOperator();

    void LexUnknown();

    SIGIL_NODISCARD bool MatchedKeyword(const std::string& identifier_buffer);

    SIGIL_NODISCARD bool IsWhitespace(char c) const;
    SIGIL_NODISCARD bool IsLineComment(char c) const;
    SIGIL_NODISCARD bool IsTerminator() const;

    void AddEOF();
};

SIGIL_NODISCARD inline std::string_view FetchTokenText(const Token token) {
    return Lexer::Source.Slice(token.offset, token.length);
}

} // namespace sigil
