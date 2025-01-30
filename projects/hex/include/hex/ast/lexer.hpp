#pragma once

#include <hex/ast/token.hpp>
#include <hex/core/type_aliases.hpp>

#include "hex/ast/lexer.hpp"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace hex {

class Lexer {
public:
    Lexer();

    void TokenizeLine(std::string_view line);
    bool Tokenize(const std::filesystem::path& file_path);
    void PrintTokens() const;
    void clear();

    HEX_NODISCARD TokenStream&& RelinquishTokens();

private:
    HEX_NODISCARD bool LexedIdentifier(std::string_view line);
    HEX_NODISCARD bool LexedString(std::string_view line);
    HEX_NODISCARD bool LexedNumber(std::string_view line);
    HEX_NODISCARD bool LexedOperator(std::string_view line);

    void LexUnknown(std::string_view line);

    HEX_NODISCARD bool MatchedKeyword(std::string& identifier_buffer);

    HEX_NODISCARD bool IsWhitespace(char c) const;
    HEX_NODISCARD bool IsComment(char c) const;

    void AddToken(TokenType type, std::string&& text);
    void AddToken(TokenType type, std::string& text);

    void AddEOF();

private:
    i64 cursor_;
    i64 line_number_;

    TokenStream token_stream_;
};

}  // namespace hex