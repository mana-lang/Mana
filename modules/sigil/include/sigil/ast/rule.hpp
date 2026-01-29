#pragma once

#include <mana/literals.hpp>

namespace sigil::ast {
namespace ml = mana::literals;

enum class Rule : ml::u8 {
    Undefined,
    Mistake,

    Artifact,

    Declaration,

    ParameterList,
    Parameter,
    FunctionDeclaration,

    DataDeclaration,
    MutableDataDeclaration,

    Statement,

    If,
    IfTail,

    Loop,
    LoopIf,
    LoopIfPost,
    LoopFixed,
    LoopRange,

    LoopBody,
    LoopRangeExpr,

    LoopControl,

    Return,

    Assignment,
    Expression,

    Scope,

    Identifier,

    Logical,
    Equality,
    Comparison,
    Term,
    Factor,
    Unary,

    Grouping,
    Literal,
    ArrayLiteral,
    ElemList,
};
} // namespace sigil::ast
