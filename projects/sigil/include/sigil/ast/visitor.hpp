#pragma once

#include <sigil/ast/nodes.hpp>

namespace sigil::ast {

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void Visit(const UnaryExpr& node)   = 0;
    virtual void Visit(const Literal_F64& node) = 0;
    virtual void Visit(const BinaryExpr& node)  = 0;
    virtual void Visit(const Module& node)      = 0;
};

}  // namespace sigil::ast