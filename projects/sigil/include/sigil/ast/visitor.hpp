#pragma once

#include <sigil/core/concepts.hpp>

namespace sigil::ast {
template <LiteralType T>
class Literal;


class Visitor {
public:
    virtual ~Visitor() = default;

    //@formatter:off
    virtual void Visit(const class Artifact& node)     = 0;
    virtual void Visit(const class Scope& node)        = 0;

    virtual void Visit(const class Datum& node)     = 0;

    virtual void Visit(const class Statement& node)    = 0;
    virtual void Visit(const class If& node)           = 0;

    virtual void Visit(const class UnaryExpr& node)    = 0;
    virtual void Visit(const class BinaryExpr& node)   = 0;

    virtual void Visit(const Literal<bool>& node)      = 0;
    virtual void Visit(const Literal<ml::i64>& node)   = 0;
    virtual void Visit(const Literal<ml::f64>& node)   = 0;
    virtual void Visit(const Literal<void>& node)      = 0;

    virtual void Visit(const class ArrayLiteral& node) = 0;
    //@formatter:on
};
} // namespace sigil::ast
