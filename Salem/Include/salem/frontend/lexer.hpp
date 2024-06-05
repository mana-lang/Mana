#pragma once

#include <salem/core/type_aliases.hpp>
#include <salem/frontend/token.hpp>

#include <vector>
#include <string>
#include <filesystem>

namespace salem {

/// TODO: add string matching
class lexer {
public:
    lexer();

    void tokenize_line(std::string_view current_line);
    bool tokenize_file(const std::filesystem::path& path_to_file);
    void print_tokens() const;
    void clear();

    SALEM_NODISCARD token_stream&& relinquish_tokens_tokens();

private:
    SALEM_NODISCARD bool lex_identifiers(std::string_view current_line);
    SALEM_NODISCARD bool lex_strings(std::string_view current_line);
    SALEM_NODISCARD bool lex_numbers(std::string_view current_line);
    SALEM_NODISCARD bool lex_operators(std::string_view current_line);
    void lex_unknown(std::string_view current_line);

    SALEM_NODISCARD bool match_keyword(std::string& ident_buffer);

    SALEM_NODISCARD bool is_whitespace(char c) const;
    SALEM_NODISCARD bool is_comment(char c) const;

    void add_token(token_type type, std::string&& text);
    void add_token(token_type type, std::string& text);

    void add_eof();

private:
    u64 cursor_;
    u64 line_number_;

    token_stream token_stream_;
};

} // namespace salem
