#pragma once

#include <mana/literals.hpp>

namespace sigil::ast {
namespace ml = mana::literals;

enum class Rule : ml::u8 {
    Undefined,
    Mistake,

    Artifact,

    Statement,

    Scope,

    Identifier,

    IfBlock,
    IfTail,

    Loop,
    LoopBody,
    LoopRangeExpr,

    Declaration,
    Assignment,
    Expression,

    Grouping,
    Literal,
    ArrayLiteral,
    ElemList,

    Unary,
    Factor,
    Term,
    Comparison,
    Equality,
    Logical,
};
} // namespace sigil::ast
