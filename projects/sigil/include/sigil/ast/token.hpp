#pragma once

#include <mana/literals.hpp>

#include <magic_enum/magic_enum.hpp>
#include <sigil/core/logger.hpp>
#include <string>
#include <vector>

namespace sigil {
namespace ml = mana::literals;
enum class TokenType : ml::i64 {
    Identifier,

    Op_Plus,
    Op_Minus,
    Op_Asterisk,
    Op_FwdSlash,

    Op_Colon,
    Op_Comma,
    Op_Assign,

    Op_BraceLeft,
    Op_BraceRight,
    Op_ParenLeft,
    Op_ParenRight,
    Op_BracketLeft,
    Op_BracketRight,

    Op_Period,
    Op_ModuleElementAccess,

    Op_LogicalNot,
    Op_Equality,
    Op_NotEqual,
    Op_LessThan,
    Op_GreaterThan,
    Op_LessEqual,
    Op_GreaterEqual,

    Op_LogicalAnd,
    Op_LogicalOr,

    Op_Arrow,

    Op_ExplicitRef,
    Op_ExplicitCopy,
    Op_ExplicitMove,

    Lit_String,
    Lit_Char,
    Lit_Int,
    Lit_Float,
    Lit_null,
    Lit_true,
    Lit_false,

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

    KW_artifact,
    KW_public,
    KW_private,
    KW_import,
    KW_as,

    KW_return,
    KW_if,
    KW_else,
    KW_match,

    KW_loop,
    KW_while,
    KW_for,
    KW_break,
    KW_skip,

    Terminator,
    Eof,

    Unknown,
    _artifact_,  // special token, auto-inserted
};

struct TextPosition {
    ml::i64 line;
    ml::i64 column;

    bool operator==(const TextPosition& other) const {
        return line == other.line && column == other.column;
    }
};

struct Token {
    TokenType    type;
    std::string  text;
    TextPosition position;

    bool operator==(const Token& other) const {
        return type == other.type && position == other.position && text == other.text;
    }

    template <typename T>
    SIGIL_NODISCARD T As() const {
        static_assert(
            false,
            "This function exists to convert literal tokens to actual literals,"
            " so only its specializations are supported"
        );
        return T();
    }
};

template <>
SIGIL_NODISCARD inline auto Token::As<ml::f32>() const -> ml::f32 {
    return std::stof(text);
}

template <>
SIGIL_NODISCARD inline auto Token::As<ml::f64>() const -> ml::f64 {
    return std::stod(text);
}

template <>
SIGIL_NODISCARD inline auto Token::As<ml::i64>() const -> ml::i64 {
    return std::stoll(text);
}

template <>
SIGIL_NODISCARD inline auto Token::As<ml::u64>() const -> ml::u64 {
    return std::stoull(text);
}

template <>
SIGIL_NODISCARD inline bool Token::As<bool>() const {
    switch (type) {
    case TokenType::Lit_true:
        return true;
    case TokenType::Lit_false:
        break;
    default:
        Log->critical(
            "Bool conversion requested for non-bool token '{}'. "
            "Defaulting to 'false'.",
            magic_enum::enum_name(type)
        );
    }

    return false;
}

using TokenStream = std::vector<Token>;

static const auto TOKEN_EOF = Token {.type = TokenType::Eof, .text = "EOF", .position = {}};

}  // namespace sigil