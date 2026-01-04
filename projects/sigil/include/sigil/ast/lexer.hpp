#pragma once

#include <sigil/ast/token.hpp>
#include <sigil/ast/source-file.hpp>

#include <mana/literals.hpp>

#include <string_view>
#include <filesystem>
#include <string>
#include <vector>

namespace sigil {
namespace ml = mana::literals;

class Lexer {
    ml::i32 cursor;

    ml::i32 line_start;
    ml::i32 line_number;

    std::vector<Token> tokens;

public:
    enum class PrintingMode {
        Print,
        Emit,
    };

    enum class PrintingPolicy {
        All,
        SkipTerminators,
    };

public:
    static thread_local GlobalSourceFile Source;

    Lexer();

    bool Tokenize(const std::filesystem::path& file_path);
    void PrintTokens(PrintingMode mode     = PrintingMode::Print,
                     PrintingPolicy policy = PrintingPolicy::All) const;
    void Reset();

    SIGIL_NODISCARD std::vector<Token>&& RelinquishTokens();

private:
    void TokenizeLine();

    SIGIL_NODISCARD ml::u16 GetTokenColumnIndex(ml::u16 token_length) const;

    void AddToken(TokenType type, ml::u16 length);

    SIGIL_NODISCARD bool LexedIdentifier();
    SIGIL_NODISCARD bool LexedString();
    SIGIL_NODISCARD bool LexedNumber();
    SIGIL_NODISCARD bool LexedOperator();

    void LexUnknown();

    SIGIL_NODISCARD bool MatchedKeyword(std::string_view identifier);

    SIGIL_NODISCARD bool IsWhitespace(char c) const;
    SIGIL_NODISCARD bool IsLineComment() const;
    SIGIL_NODISCARD bool IsTerminator() const;

    void AddEOF();
};
} // namespace sigil
