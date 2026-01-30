#pragma once

#include <string_view>
#include <sigil/ast/token.hpp>

#include <unordered_map>
#include <array>

namespace sigil {
namespace ml = mana::literals;

enum class PrimitiveType : ml::u8 {
    I8, I16, I32, I64, Isize,
    U8, U16, U32, U64, Usize,
    F32, F64,
    Char, String,
    Byte, Bool,
    Fn,
    None,
};

inline constexpr ml::u8 NUM_PRIMITIVES = 18;

inline constexpr std::array<std::string_view, NUM_PRIMITIVES> PRIMITIVES = {
    "i8",
    "i16",
    "i32",
    "i64",
    "isize",

    "u8",
    "u16",
    "u32",
    "u64",
    "usize",

    "f32",
    "f64",
    "char",
    "string",
    "byte",
    "bool",
    "fn",

    "none"
};

consteval ml::u8 PrimitiveIndex(PrimitiveType type) {
    return static_cast<ml::u8>(type);
}

consteval std::string_view PrimitiveName(PrimitiveType type) {
    return PRIMITIVES[PrimitiveIndex(type)];
}

using KeywordMap = std::unordered_map<std::string_view, TokenType>;
// @formatter:off
inline const KeywordMap KEYWORDS = {
    {PrimitiveName(PrimitiveType::I8),        TokenType::KW_i8         },
    {PrimitiveName(PrimitiveType::I16),       TokenType::KW_i16        },
    {PrimitiveName(PrimitiveType::I32),       TokenType::KW_i32        },
    {PrimitiveName(PrimitiveType::I64),       TokenType::KW_i64        },
    {PrimitiveName(PrimitiveType::Isize),     TokenType::KW_isize      },

    {PrimitiveName(PrimitiveType::U8),        TokenType::KW_u8         },
    {PrimitiveName(PrimitiveType::U16),       TokenType::KW_u16        },
    {PrimitiveName(PrimitiveType::U32),       TokenType::KW_u32        },
    {PrimitiveName(PrimitiveType::U64),       TokenType::KW_u64        },
    {PrimitiveName(PrimitiveType::Usize),     TokenType::KW_usize      },

    {PrimitiveName(PrimitiveType::F32),       TokenType::KW_f32        },
    {PrimitiveName(PrimitiveType::F64),       TokenType::KW_f64        },

    {PrimitiveName(PrimitiveType::Char),      TokenType::KW_char       },
    {PrimitiveName(PrimitiveType::String),    TokenType::KW_string     },

    {PrimitiveName(PrimitiveType::Byte),      TokenType::KW_byte       },
    {PrimitiveName(PrimitiveType::Bool),      TokenType::KW_bool       },
    {PrimitiveName(PrimitiveType::Fn),        TokenType::KW_fn         },

    {PrimitiveName(PrimitiveType::None),      TokenType::Lit_none      },

    {"data",      TokenType::KW_data       },
    {"mut",       TokenType::KW_mut        },
    {"const",     TokenType::KW_const      },

    {"type",      TokenType::KW_type       },
    {"Tag",       TokenType::KW_Tag        },
    {"enum",      TokenType::KW_enum       },
    {"variant",   TokenType::KW_variant    },
    {"interface", TokenType::KW_interface  },

    {"module",    TokenType::KW_module     },
    {"public",    TokenType::KW_public     },
    {"private",   TokenType::KW_private    },
    {"import",    TokenType::KW_import     },
    {"as",        TokenType::KW_as         },

    {"return",    TokenType::KW_return     },
    {"true",      TokenType::Lit_true      },
    {"false",     TokenType::Lit_false     },
    {"if",        TokenType::KW_if         },
    {"else",      TokenType::KW_else       },
    {"match",     TokenType::KW_match      },

    {"loop",      TokenType::KW_loop       },
    {"for",       TokenType::KW_for        },
    {"in",        TokenType::KW_in         },
    {"break",     TokenType::KW_break      },
    {"skip",      TokenType::KW_skip       },

    {"when",      TokenType::KW_when       },

    {"and",       TokenType::Op_LogicalAnd },
    {"or",        TokenType::Op_LogicalOr  },
    {"not",       TokenType::Op_LogicalNot },
};
// @formatter:on
} // namespace sigil
