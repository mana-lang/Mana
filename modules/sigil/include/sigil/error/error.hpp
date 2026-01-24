#pragma once
#include <mana/literals.hpp>
#include <sigil/error/error-code.hpp>

namespace sigil {
namespace ml = mana::literals;

struct Error {
    const class ParseNode* node;
    ml::i64 token_offset;

    ErrorCode code;
    ErrorSeverity severity;

    Error(const ParseNode* node,
          const ErrorCode code,
          const ml::i64 token_offset,
          const ErrorSeverity severity = ErrorSeverity::Error
    )
        : node(node),
          token_offset(token_offset),
          code(code),
          severity(severity) {}
};
} // namespace sigil
