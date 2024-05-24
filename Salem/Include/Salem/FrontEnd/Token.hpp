#pragma once

#include <Salem/Core/TypeAliases.hpp>

#include <string>

namespace salem {

struct TextPosition {
    u64 line;
    u64 column;
};

struct Token {
    enum class Type;

    Type type;
    std::string contents;
    TextPosition position;
};

enum class Token::Type {
    Int,
    Float,

    Identifier,

    Op_Add,
    Op_Sub,
    Op_Mul,
    Op_Div,

    Op_Colon,
    Op_Comma,
    Op_Assign,

    Op_BraceLeft,
    Op_BraceRight,
    Op_ParenLeft,
    Op_ParenRight,
    Op_BracketLeft,
    Op_BracketRight,

    KW_i8,
    KW_i16,
    KW_i32,
    KW_i64,
    KW_i128,

    KW_u8,
    KW_u16,
    KW_u32,
    KW_u64,
    KW_u128,

    KW_f32,
    KW_f64,

    KW_byte,
    KW_char,
    KW_string,

    KW_bool,
    KW_void,

    KW_data,
    KW_fn,
    KW_mut,
    KW_const,
    KW_raw,
    KW_override,

    KW_pack,
    KW_struct,
    KW_enum,
    KW_generic,

    KW_module,
    KW_public,
    KW_private,
    KW_import,
    KW_as,

    KW_return,
    KW_true,
    KW_false,
    KW_if,
    KW_else,
    KW_match,

    KW_loop,
    KW_while,
    KW_for,
    KW_break,
    KW_skip,

    Newline,
    Eof,

    Unknown,
};

}  // namespace salem