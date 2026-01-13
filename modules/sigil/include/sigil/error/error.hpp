#pragma once
#include <mana/literals.hpp>
#include <sigil/error/error-code.hpp>

namespace sigil {
namespace ml = mana::literals;

struct Error {
    const class ParseNode* node;

    ErrorCode code;
    ErrorSeverity severity;
    ml::i64 token_offset;

    Error(
        const ParseNode* node,
        const ErrorCode code,
        const ml::i64 token_offset,
        const ErrorSeverity severity = ErrorSeverity::Error
    )
        : node(node),
          code(code),
          severity(severity),
          token_offset(token_offset) {}
};
} // namespace sigil
