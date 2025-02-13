#pragma once

#include <sigil/ast/nodes.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace circe {
namespace ml = mana::literals;

class MainVisitor final : public sigil::ast::Visitor {
    mana::vm::Slice slice;

public:
    mana::vm::Slice GetSlice() const;

    void Visit(const sigil::ast::UnaryExpr& node) override;
    void Visit(const sigil::ast::Artifact& artifact) override;
    void Visit(const sigil::ast::BinaryExpr& node) override;
    void Visit(const sigil::ast::Literal<ml::f64>& node) override;
    void Visit(const sigil::ast::Literal<ml::i64>& node) override;
    void Visit(const sigil::ast::Literal<void>& node) override;
    void Visit(const sigil::ast::Literal<bool>& node) override;
};

}  // namespace circe
