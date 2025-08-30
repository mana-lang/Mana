#pragma once

#include <mana/literals.hpp>
#include <sigil/ast/token.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace sigil {
namespace ml = mana::literals;

struct TokenizedSource {
    std::string source;
    std::vector<Token> tokens;
};

class Lexer {
    ml::i64 cursor;

    ml::i64 line_start;
    ml::i64 line_number;

    TokenStream token_stream;
    std::string source;
public:
    Lexer();

    bool Tokenize(const std::filesystem::path& file_path);
    void PrintTokens() const;
    void Reset();

    SIGIL_NODISCARD TokenStream&& RelinquishTokens();

private:
    void TokenizeLine();

    mana::literals::i64  GetTokenColumnIndex(std::size_t token_length);

    void AddToken(TokenType type, std::string&& text);
    void AddToken(TokenType type, std::string& text);
    void AddToken(TokenType type, char c);

    SIGIL_NODISCARD bool LexedIdentifier();
    SIGIL_NODISCARD bool LexedString();
    SIGIL_NODISCARD bool LexedNumber();
    SIGIL_NODISCARD bool LexedOperator();

    void LexUnknown();

    SIGIL_NODISCARD bool MatchedKeyword(std::string& identifier_buffer);

    SIGIL_NODISCARD bool IsWhitespace(char c) const;
    SIGIL_NODISCARD bool IsLineComment(char c) const;
    SIGIL_NODISCARD bool IsTerminator() const;

    void AddEOF();
};

}  // namespace sigil