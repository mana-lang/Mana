#pragma once

#include <sigil/core/concepts.hpp>

namespace sigil::ast {
template <LiteralType T>
class Literal;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void Visit(const class Artifact& node) = 0;
    virtual void Visit(const class Scope& node) = 0;

    virtual void Visit(const class DataDeclaration& node) = 0;
    virtual void Visit(const class Identifier& node) = 0;
    virtual void Visit(const class Assignment& node) = 0;

    virtual void Visit(const class If& node) = 0;

    virtual void Visit(const class Loop& node) = 0;
    virtual void Visit(const class LoopIf& node) = 0;
    virtual void Visit(const class LoopIfPost& node) = 0;
    virtual void Visit(const class LoopRange& node) = 0;
    virtual void Visit(const class LoopFixed& node) = 0;

    virtual void Visit(const class Break& node) = 0;
    virtual void Visit(const class Skip& node) = 0;

    virtual void Visit(const class UnaryExpr& node) = 0;
    virtual void Visit(const class BinaryExpr& node) = 0;

    virtual void Visit(const class ArrayLiteral& node) = 0;

    virtual void Visit(const Literal<bool>& node) = 0;
    virtual void Visit(const Literal<ml::i64>& node) = 0;
    virtual void Visit(const Literal<ml::f64>& node) = 0;
    virtual void Visit(const Literal<void>& node) = 0;
};
} // namespace sigil::ast
