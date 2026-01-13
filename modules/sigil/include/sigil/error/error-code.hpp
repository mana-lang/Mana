#pragma once

namespace sigil {
enum class ErrorCode {
    Unknown,

    Grouping_NoExpr,
};

enum class ErrorSeverity {
    Hint,
    Suggestion,
    Warning,
    Error,
};
} // namespace sigil
