#pragma once

#include <mana/literals.hpp>
#include <type_traits>

namespace sigil::ast {
using namespace mana::literals;

template <typename T> requires std::is_arithmetic_v<T>
class Literal;

class Initializer;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void Visit(const class Artifact&) = 0;
    virtual void Visit(const class Scope&) = 0;

    virtual void Visit(const class FunctionDeclaration&) = 0;
    virtual void Visit(const class MutableDataDeclaration&) = 0;
    virtual void Visit(const class DataDeclaration&) = 0;

    virtual void Visit(const class Identifier&) = 0;
    virtual void Visit(const class Assignment&) = 0;

    virtual void Visit(const class Return&) = 0;
    virtual void Visit(const class Invocation&) = 0;

    virtual void Visit(const class If&) = 0;

    virtual void Visit(const class Loop&) = 0;
    virtual void Visit(const class LoopIf&) = 0;
    virtual void Visit(const class LoopIfPost&) = 0;
    virtual void Visit(const class LoopFixed&) = 0;
    virtual void Visit(const class LoopRange&) = 0;
    virtual void Visit(const class LoopRangeMutable&) = 0;

    virtual void Visit(const class Break&) = 0;
    virtual void Visit(const class Skip&) = 0;

    virtual void Visit(const class UnaryExpr&) = 0;
    virtual void Visit(const class BinaryExpr&) = 0;

    virtual void Visit(const class ListExpression&) = 0;

    virtual void Visit(const class ListAccess&) = 0;

    virtual void Visit(const Literal<bool>&) = 0;
    virtual void Visit(const Literal<i64>&) = 0;
    virtual void Visit(const Literal<f64>&) = 0;

    virtual void Visit(const class StringLiteral&) = 0;
};
} // namespace sigil::ast
