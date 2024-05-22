#ifndef SALEM_LEXER_HPP
#define SALEM_LEXER_HPP

#include <Salem/Core/TypeAliases.hpp>
#include <Salem/FrontEnd/Token.hpp>

#include <vector>
#include <string>
#include <filesystem>

namespace salem {

class Lexer {
public:
    Lexer();

    bool TokenizeFile(const std::filesystem::path& path_to_file);
    void PrintTokens() const;

    SALEM_NODISCARD auto RelinquishTokens() -> std::vector<Token>&&;

private:
    bool LexIdentifiers(std::string_view current_line);
    bool LexNumbers(std::string_view current_line);
    bool LexOperators(std::string_view current_line);
    void LexUnknown(std::string_view current_line);

    SALEM_NODISCARD bool IsWhitespace(char c);
    SALEM_NODISCARD bool IsComment(char c);

    void AddToken(Token::Type type, const std::string&& contents);

private:
    u64 cursor_;
    u64 line_number_;

    std::vector<Token> token_stream_;
};

}  // namespace salem

#endif