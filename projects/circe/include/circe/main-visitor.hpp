#pragma once

#include <sigil/ast/nodes.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/vm/slice.hpp>

namespace circe {

class MainVisitor final : public sigil::ast::Visitor {
    mana::vm::Slice slice;

public:
    mana::vm::Slice GetSlice() const;

    void Visit(const sigil::ast::UnaryExpr& node) override;
    void Visit(const sigil::ast::Module& module) override;
    void Visit(const sigil::ast::BinaryExpr& node) override;
    void Visit(const sigil::ast::Literal_F64& node) override;
};

}  // namespace circe
