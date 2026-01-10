#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace circe {
namespace ml = mana::literals;
namespace mv = mana::vm;
namespace ast = sigil::ast;

class CirceVisitor final : public ast::Visitor {
    struct Symbol {
        ml::u16 register_index;
        ml::u16 scope_depth;
    };
    using SymbolTable = std::unordered_map<std::string, Symbol>;

    mv::Slice   slice;
    SymbolTable symbols;
    ml::u16     total_registers;
    ml::u16     scope_depth;

    std::vector<ml::u16> reg_buffer;
    std::vector<ml::u16> free_regs;

public:
    CirceVisitor();

    CIRCE_NODISCARD mv::Slice GetSlice() const;

    void Visit(const ast::Artifact& artifact) override;

    void Visit(const ast::UnaryExpr& node) override;
    void Visit(const ast::BinaryExpr& node) override;

    void Visit(const ast::Statement& node) override;
    void Visit(const ast::Scope& node) override;
    void Visit(const ast::DataDeclaration& node) override;
    void Visit(const ast::Identifier& node) override;

    void Visit(const ast::If& node) override;

    void Visit(const ast::Literal<ml::f64>& literal) override;
    void Visit(const ast::Literal<ml::i64>& literal) override;
    void Visit(const ast::Literal<void>& node) override;
    void Visit(const ast::Literal<bool>& literal) override;
    void Visit(const ast::ArrayLiteral& array) override;


private:
    ml::u16 AllocateRegister();
    void FreeRegister(ml::u16 reg);
    void FreeRegisters(std::initializer_list<ml::u16> regs);
    void FreeRegisters(const std::vector<ml::u16>& regs);

    bool RegisterIsOwned(ml::u16 reg);

    ml::u16 PopRegBuffer();
    void ClearRegBuffer();

    void CleanupCurrentScope();

    void AddSymbol(const std::string& name, ml::u16 register_index);
    void RemoveSymbol(const std::string& name);

    template <typename T>
    void CreateLiteral(const ast::Literal<T>& literal) {
        ml::u16 dst = AllocateRegister();
        ml::u16 idx = slice.AddConstant(literal.Get());
        slice.Write(mv::Op::LoadConstant, {dst, idx});

        reg_buffer.push_back(dst);
    }
};
} // namespace circe
