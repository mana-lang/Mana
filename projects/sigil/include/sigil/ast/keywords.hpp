#pragma once

#include <string_view>
#include <sigil/ast/token.hpp>

#include <unordered_map>

namespace sigil {
using KeywordMap = std::unordered_map<std::string_view, TokenType>;

// @formatter:off
static const KeywordMap keyword_map = {
    {"i8",        TokenType::KW_i8         },
    {"i16",       TokenType::KW_i16        },
    {"i32",       TokenType::KW_i32        },
    {"i64",       TokenType::KW_i64        },
    {"isize",     TokenType::KW_isize      },

    {"u8",        TokenType::KW_u8         },
    {"u16",       TokenType::KW_u16        },
    {"u32",       TokenType::KW_u32        },
    {"u64",       TokenType::KW_u64        },
    {"usize",     TokenType::KW_usize      },

    {"f32",       TokenType::KW_f32        },
    {"f64",       TokenType::KW_f64        },

    {"char",      TokenType::KW_char       },
    {"string",    TokenType::KW_string     },

    {"byte",      TokenType::KW_byte       },
    {"bool",      TokenType::KW_bool       },
    {"none",      TokenType::Lit_none      },

    {"data",      TokenType::KW_data       },
    {"fn",        TokenType::KW_fn         },
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
