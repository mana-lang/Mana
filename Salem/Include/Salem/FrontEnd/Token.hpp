#pragma once

#include <Salem/Core/TypeAliases.hpp>

#include <string>

namespace salem {

struct TextPosition {
    u64 line;
    u64 column;
};

struct Token {
    enum class Type : u8;

    Type type;
    std::string contents;
    TextPosition position;
};

enum class Token::Type : u8 {
    Int,
    Float,

    Identifier,

    OpAdd,
    OpSub,
    OpMul,
    OpDiv,

    OpColon,
    OpComma,
    OpAssign,

    OpBraceLeft,
    OpBraceRight,
    OpParenLeft,
    OpParenRight,
    OpBracketLeft,
    OpBracketRight,

    Newline,
    Eof,

    Unknown,
};

}  // namespace salem