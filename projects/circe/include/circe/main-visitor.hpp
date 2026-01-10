#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace circe {
namespace ml = mana::literals;
namespace mv = mana::vm;
namespace ast = sigil::ast;

class MainVisitor final : public ast::Visitor {
    using SymbolTable = std::unordered_map<std::string, ml::u16>;

    mv::Slice   slice;
    SymbolTable symbols;
    ml::u16     next_slot = {};

    std::vector<ml::u16> reg_stack;

public:
    CIRCE_NODISCARD mv::Slice GetSlice() const;
    CIRCE_NODISCARD ml::u64   ComputeJumpDist(ml::u64 index) const;

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
    void Visit(const ast::DataDeclaration& node) override;
    void Visit(const ast::Identifier& node) override;

private:
    ml::u16 AllocateRegister();
    ml::u16 PopRegStack();

    template <typename T>
    void CreateLiteral(const ast::Literal<T>& literal) {
        ml::u16 dst = AllocateRegister();
        ml::u16 idx = slice.AddConstant(literal.Get());
        slice.Write(mv::Op::LoadConstant, {dst, idx});

        reg_stack.push_back(dst);
    }
};
} // namespace circe
