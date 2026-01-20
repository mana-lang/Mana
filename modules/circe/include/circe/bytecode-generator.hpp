#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/hexe.hpp>

namespace circe {
namespace ml = mana::literals;
namespace mv = mana::vm;
namespace ast = sigil::ast;

class BytecodeGenerator final : public ast::Visitor {
    struct Symbol {
        ml::u16 register_index;
        ml::u8 scope_depth;
        bool is_mutable;
    };

    struct JumpInstruction {
        ml::i64 jump_index;
        bool is_conditional;
    };

    struct LoopContext {
        ml::i64 start_addr {ml::SENTINEL_64};

        std::vector<JumpInstruction> pending_breaks;
        std::vector<JumpInstruction> pending_skips;
    };

    using SymbolTable = std::unordered_map<std::string_view, Symbol>;

    mv::Hexe output;
    SymbolTable symbols;
    ml::u16 total_registers;
    ml::u8 scope_depth;

    std::vector<ml::u16> reg_buffer;
    std::vector<ml::u16> free_regs;

    std::vector<LoopContext> loop_stack;

public:
    BytecodeGenerator();

    CIRCE_NODISCARD mv::Hexe Bytecode() const;

    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::Scope& node) override;

    void Visit(const ast::MutableDataDeclaration& node) override;
    void Visit(const ast::DataDeclaration& node) override;
    void Visit(const ast::Identifier& node) override;
    void Visit(const ast::Assignment& node) override;

    void Visit(const ast::If& node) override;

    void Visit(const ast::Loop& node) override;
    void Visit(const ast::LoopIf& node) override;
    void Visit(const ast::LoopIfPost& node) override;
    void Visit(const ast::LoopRange& node) override;
    void Visit(const ast::LoopFixed& node) override;

    void Visit(const ast::Break& node) override;
    void Visit(const ast::Skip& node) override;

    void Visit(const ast::UnaryExpr& node) override;
    void Visit(const ast::BinaryExpr& node) override;
    void Visit(const ast::ArrayLiteral& array) override;

    void Visit(const ast::Literal<ml::f64>& literal) override;
    void Visit(const ast::Literal<ml::i64>& literal) override;
    void Visit(const ast::Literal<void>& node) override;
    void Visit(const ast::Literal<bool>& literal) override;

private:
    ml::u16 CalcJumpDistance(ml::i64 jump_index, bool is_conditional = false) const;
    ml::u16 CalcJumpBackwards(ml::i64 target_index,
                              ml::i64 source_index,
                              bool is_conditional = false
    ) const;

    ml::u16 AllocateRegister();
    void FreeRegister(ml::u16 reg);
    void FreeRegisters(std::initializer_list<ml::u16> regs);
    void FreeRegisters(const std::vector<ml::u16>& regs);

    bool RegisterIsOwned(ml::u16 reg);

    ml::u16 PopRegBuffer();
    void ClearRegBuffer();

    void EnterScope();
    void ExitScope();

    void AddSymbol(std::string_view name, ml::u16 register_index, bool is_mutable);
    void RemoveSymbol(std::string_view name);

    LoopContext& CurrentLoop();
    void HandleLoopControl(bool is_break, const ast::NodePtr& condition);

    void HandleDeclaration(const ast::Binding& node, bool is_mutable);

    template <typename T>
    void CreateLiteral(const ast::Literal<T>& literal) {
        ml::u16 dst = AllocateRegister();
        ml::u16 idx = output.AddConstant(literal.Get());
        output.Write(mv::Op::LoadConstant, {dst, idx});

        reg_buffer.push_back(dst);
    }
};
} // namespace circe
