#pragma once

namespace sigil::ast {

template <typename T>
class Literal;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void Visit(const class UnaryExpr& node)  = 0;
    virtual void Visit(const class BinaryExpr& node) = 0;
    virtual void Visit(const class Artifact& node)   = 0;

    virtual void Visit(const Literal<bool>& node)    = 0;
    virtual void Visit(const Literal<ml::i64>& node) = 0;
    virtual void Visit(const Literal<ml::f64>& node) = 0;
    virtual void Visit(const Literal<void>& node)    = 0;

    virtual void Visit(const class ArrayLiteral& node) = 0;
};

}  // namespace sigil::ast