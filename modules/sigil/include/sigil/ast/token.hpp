#pragma once

#include <mana/literals.hpp>

namespace sigil {
namespace ml = mana::literals;

enum class TokenType : ml::u8 {
    Identifier,

    Op_Plus,
    Op_Minus,
    Op_Asterisk,
    Op_FwdSlash,
    Op_Modulo,

    Op_AddAssign,
    Op_SubAssign,
    Op_MulAssign,
    Op_DivAssign,
    Op_ModAssign,

    Op_Colon,
    Op_Comma,
    Op_Assign,

    Op_BraceLeft,
    Op_BraceRight,
    Op_ParenLeft,
    Op_ParenRight,
    Op_BracketLeft,
    Op_BracketRight,

    Op_Access,
    Op_Range,
    Op_ScopeResolution,

    Op_LogicalNot,
    Op_Equality,
    Op_NotEqual,
    Op_LessThan,
    Op_GreaterThan,
    Op_LessEqual,
    Op_GreaterEqual,

    Op_LogicalAnd,
    Op_LogicalOr,

    Op_Binding,
    Op_MultiMatch,

    Op_ReturnType,
    Op_Attribute,

    Op_Ref,
    Op_Move,
    Op_Copy,

    Lit_String,
    Lit_Char,
    Lit_Int,
    Lit_Float,

    Lit_none,
    Lit_true,
    Lit_false,

    KW_i8,
    KW_i16,
    KW_i32,
    KW_i64,
    KW_isize,

    KW_u8,
    KW_u16,
    KW_u32,
    KW_u64,
    KW_usize,

    KW_f32,
    KW_f64,

    KW_byte,
    KW_char,
    KW_string,
    KW_bool,

    KW_data,
    KW_fn,
    KW_mut,
    KW_const,

    KW_type,
    KW_Tag,
    KW_enum,
    KW_variant,
    KW_interface,

    KW_module,
    KW_public,
    KW_private,
    KW_import,
    KW_as,

    KW_return,
    KW_if,
    KW_else,
    KW_match,

    KW_loop,
    KW_for,
    KW_in,
    KW_break,
    KW_skip,

    KW_when,

    Terminator,
    Eof,

    Unknown,
};

struct Token {
    ml::i32 line;
    ml::i32 offset;
    ml::u16 column;
    ml::u16 length;

    TokenType type;

    bool operator==(const Token& other) const {
        return type == other.type
               && offset == other.offset
               && length == other.length
               && line == other.line
               && column == other.column;
    }
};
} // namespace sigil
