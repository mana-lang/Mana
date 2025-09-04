#pragma once

#include <sigil/ast/token.hpp>

#include <unordered_map>

namespace sigil {

using KeywordMap = std::unordered_map<const char*, TokenType>;

static const KeywordMap keyword_map = {
    {"i8",       TokenType::KW_i8        },
    {"i16",      TokenType::KW_i16       },
    {"i32",      TokenType::KW_i32       },
    {"i64",      TokenType::KW_i64       },
    {"i128",     TokenType::KW_i128      },

    {"u8",       TokenType::KW_u8        },
    {"u16",      TokenType::KW_u16       },
    {"u32",      TokenType::KW_u32       },
    {"u64",      TokenType::KW_u64       },
    {"u128",     TokenType::KW_u128      },

    {"f32",      TokenType::KW_f32       },
    {"f64",      TokenType::KW_f64       },

    {"byte",     TokenType::KW_byte      },
    {"char",     TokenType::KW_char      },
    {"string",   TokenType::KW_string    },

    {"bool",     TokenType::KW_bool      },
    {"null",     TokenType::Lit_null     },

    {"data",     TokenType::KW_data      },
    {"fn",       TokenType::KW_fn        },
    {"mut",      TokenType::KW_mut       },
    {"const",    TokenType::KW_const     },
    {"override", TokenType::KW_override  },

    {"pack",     TokenType::KW_type      },
    {"struct",   TokenType::KW_struct    },
    {"enum",     TokenType::KW_enum      },
    {"generic",  TokenType::KW_generic   },

    {"module",   TokenType::KW_artifact  },
    {"public",   TokenType::KW_public    },
    {"private",  TokenType::KW_private   },
    {"import",   TokenType::KW_import    },
    {"as",       TokenType::KW_as        },

    {"return",   TokenType::KW_return    },
    {"true",     TokenType::Lit_true     },
    {"false",    TokenType::Lit_false    },
    {"if",       TokenType::KW_if        },
    {"else",     TokenType::KW_else      },
    {"match",    TokenType::KW_match     },

    {"loop",     TokenType::KW_loop      },
    {"while",    TokenType::KW_while     },
    {"for",      TokenType::KW_for       },
    {"break",    TokenType::KW_break     },
    {"skip",     TokenType::KW_skip      },

    {"and",      TokenType::Op_LogicalAnd},
    {"or",       TokenType::Op_LogicalOr },
    {"not",      TokenType::Op_LogicalNot},
};

}  // namespace sigil