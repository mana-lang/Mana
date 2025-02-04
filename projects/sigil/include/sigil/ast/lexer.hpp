#pragma once

#include <sigil/core/type_aliases.hpp>
#include <sigil/ast/token.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace sigil {

class Lexer {
public:
    Lexer();

    void TokenizeLine(std::string_view line);
    bool Tokenize(const std::filesystem::path& file_path);
    void PrintTokens() const;
    void clear();

    SIGIL_NODISCARD TokenStream&& RelinquishTokens();

private:
    SIGIL_NODISCARD bool LexedIdentifier(std::string_view line);
    SIGIL_NODISCARD bool LexedString(std::string_view line);
    SIGIL_NODISCARD bool LexedNumber(std::string_view line);
    SIGIL_NODISCARD bool LexedOperator(std::string_view line);

    void LexUnknown(std::string_view line);

    SIGIL_NODISCARD bool MatchedKeyword(std::string& identifier_buffer);

    SIGIL_NODISCARD bool IsWhitespace(char c) const;
    SIGIL_NODISCARD bool IsComment(char c) const;

    void AddToken(TokenType type, std::string&& text);
    void AddToken(TokenType type, std::string& text);

    void AddEOF();

private:
    i64 cursor_;
    i64 line_number_;

    TokenStream token_stream_;
};

}  // namespace sigil