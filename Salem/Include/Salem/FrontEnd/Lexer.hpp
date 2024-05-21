#pragma once

#include <Salem/Core/TypeAliases.hpp>
#include <Salem/FrontEnd/Token.hpp>

#include <vector>
#include <string>
#include <filesystem>

namespace salem {

class Lexer {
public:
    Lexer();

    bool tokenize_file(const std::filesystem::path& path_to_file);

    SALEM_NODISCARD auto relinquish_tokens() -> std::vector<Token>&&;

private:
    bool lex_identifiers(std::string_view current_line);
    bool lex_numbers(std::string_view current_line);
    bool lex_operators(std::string_view current_line);
    void lex_unknown(std::string_view current_line);

    SALEM_NODISCARD static bool is_whitespace(char c);
    SALEM_NODISCARD static bool is_comment(char c);

    void add_token(Token::Type type, const std::string&& contents);

private:
    u64 cursor_;
    u64 line_number_;

    std::vector<Token> token_stream_;
};

}  // namespace salem