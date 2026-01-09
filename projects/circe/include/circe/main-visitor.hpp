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

    std::unordered_map<std::string, ml::u16> symbols;
    ml::u16 next_slot = {};

public:
    CIRCE_NODISCARD mana::vm::Slice GetSlice() const;
    CIRCE_NODISCARD ml::u64 ComputeJumpDist(mana::literals::u64 index) const;

    void Visit(const ast::UnaryExpr& node) override;
    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::BinaryExpr& node) override;
    void Visit(const ast::Literal<ml::f64>& literal) override;
    void Visit(const ast::Literal<ml::i64>& literal) override;
    void Visit(const ast::Literal<void>& node) override;
    void Visit(const ast::Literal<bool>& literal) override;
    void Visit(const ast::ArrayLiteral& array) override;
    void Visit(const ast::Statement& node) override;
    void Visit(const ast::Scope& node) override;
    void Visit(const ast::If& node) override;
    void Visit(const ast::Datum& node) override;
    void Visit(const ast::Identifier& node) override;
};

}  // namespace circe
