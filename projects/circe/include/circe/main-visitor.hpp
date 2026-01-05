#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace circe {
namespace ml = mana::literals;
namespace ast = sigil::ast;

class MainVisitor final : public ast::Visitor {
    mana::vm::Slice slice;

public:
    CIRCE_NODISCARD mana::vm::Slice GetSlice() const;

    void Visit(const ast::UnaryExpr& node) override;
    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::BinaryExpr& node) override;
    void Visit(const ast::Literal<ml::f64>& node) override;
    void Visit(const ast::Literal<ml::i64>& node) override;
    void Visit(const ast::Literal<void>& node) override;
    void Visit(const ast::Literal<bool>& node) override;
    void Visit(const ast::ArrayLiteral& node) override;
    void Visit(const ast::Statement& node) override;
    void Visit(const ast::Scope& node) override;
    void Visit(const ast::If& node) override;
};

}  // namespace circe
