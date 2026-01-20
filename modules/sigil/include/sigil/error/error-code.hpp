#pragma once

namespace sigil {
enum class ErrorCode {
    Unknown,

    Grouping_NoExpr,


    // Semantic errors
    Semantic_UndefinedVariable,
    Semantic_RedeclaredVariable,
    Semantic_AssignToImmutable,
    Semantic_AssignToUndefined,
    Semantic_TypeMismatch,
    Semantic_BreakOutsideLoop,
    Semantic_SkipOutsideLoop,
    Semantic_InvalidConditionType,
    Semantic_UseAfterMove,
};

enum class ErrorSeverity {
    Hint,
    Suggestion,
    Warning,
    Error,
};
} // namespace sigil