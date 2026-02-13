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

    virtual void Visit(const class Artifact& node) = 0;
    virtual void Visit(const class Scope& node) = 0;

    virtual void Visit(const class FunctionDeclaration& node) = 0;
    virtual void Visit(const class MutableDataDeclaration& node) = 0;
    virtual void Visit(const class DataDeclaration& node) = 0;

    virtual void Visit(const class Identifier& node) = 0;
    virtual void Visit(const class Assignment& node) = 0;

    virtual void Visit(const class Return& node) = 0;
    virtual void Visit(const class Invocation& node) = 0;

    virtual void Visit(const class If& node) = 0;

    virtual void Visit(const class Loop& node) = 0;
    virtual void Visit(const class LoopIf& node) = 0;
    virtual void Visit(const class LoopIfPost& node) = 0;
    virtual void Visit(const class LoopFixed& node) = 0;
    virtual void Visit(const class LoopRange& node) = 0;
    virtual void Visit(const class LoopRangeMutable& node) = 0;

    virtual void Visit(const class Break& node) = 0;
    virtual void Visit(const class Skip& node) = 0;

    virtual void Visit(const class UnaryExpr& node) = 0;
    virtual void Visit(const class BinaryExpr& node) = 0;

    virtual void Visit(const class ListLiteral& node) = 0;

    virtual void Visit(const Literal<bool>& node) = 0;
    virtual void Visit(const Literal<i64>& node) = 0;
    virtual void Visit(const Literal<f64>& node) = 0;

    virtual void Visit(const class StringLiteral& node) = 0;
};
} // namespace sigil::ast
