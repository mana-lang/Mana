#pragma once

#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

namespace sigil::ast {
namespace ml = mana::literals;

class SemanticAnalyzer final : public Visitor {
public:
    void Visit(const Artifact& artifact) override;
    void Visit(const Scope& node) override;

    void Visit(const MutableDataDeclaration& node) override;
    void Visit(const DataDeclaration& node) override;
    void Visit(const Identifier& node) override;
    void Visit(const Assignment& node) override;

    void Visit(const If& node) override;

    void Visit(const Loop& node) override;
    void Visit(const LoopIf& node) override;
    void Visit(const LoopIfPost& node) override;
    void Visit(const LoopRange& node) override;
    void Visit(const LoopFixed& node) override;

    void Visit(const Break& node) override;
    void Visit(const Skip& node) override;

    void Visit(const UnaryExpr& node) override;
    void Visit(const BinaryExpr& node) override;
    void Visit(const ArrayLiteral& array) override;

    void Visit(const Literal<ml::f64>& literal) override;
    void Visit(const Literal<ml::i64>& literal) override;
    void Visit(const Literal<void>& node) override;
    void Visit(const Literal<bool>& literal) override;
};
} // namespace sigil::ast
