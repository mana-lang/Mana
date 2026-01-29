#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <mana/vm/bytecode.hpp>

namespace circe {
namespace ml = mana::literals;
namespace mv = mana::vm;
namespace ast = sigil::ast;

using Register = ml::u16;

class BytecodeGenerator final : public ast::Visitor {
    using ScopeDepth = ml::u8;

    struct Symbol {
        Register register_index;
        ScopeDepth scope_depth;
    };

    struct Constant {
        Register register_index;
        ScopeDepth scope_depth;
    };

    struct JumpInstruction {
        ml::i64 jump_index;
        bool is_conditional;
    };

    struct LoopContext {
        std::vector<JumpInstruction> pending_breaks;
        std::vector<JumpInstruction> pending_skips;
    };

    using SymbolTable   = std::unordered_map<std::string_view, Symbol>;
    using ConstantTable = std::unordered_map<ml::u16, Constant>;

    SymbolTable symbols;
    ConstantTable constants;

    ml::u16 total_registers;
    ScopeDepth scope_depth;

    std::vector<Register> reg_buffer;
    std::vector<Register> free_regs;

    std::vector<LoopContext> loop_stack;

    mv::ByteCode bytecode;

public:
    BytecodeGenerator();

    CIRCE_NODISCARD mv::ByteCode Bytecode() const;

    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::Scope& node) override;

    void Visit(const ast::FunctionDeclaration& node) override {}
    void Visit(const ast::MutableDataDeclaration& node) override;
    void Visit(const ast::DataDeclaration& node) override;

    void Visit(const ast::Identifier& node) override;
    void Visit(const ast::Assignment& node) override;

    void Visit(const ast::Return& node) override {}

    void Visit(const ast::If& node) override;

    void Visit(const ast::Loop& node) override;
    void Visit(const ast::LoopIf& node) override;
    void Visit(const ast::LoopIfPost& node) override;
    void Visit(const ast::LoopFixed& node) override;
    void Visit(const ast::LoopRange& node) override;
    void Visit(const ast::LoopRangeMutable& node) override;

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
    bool IsConditionalJumpOp(mv::Op op) const;
    void JumpBackwards(ml::i64 target_index);
    void JumpBackwardsConditional(mv::Op op, Register condition_register, ml::i64 target_index);
    void PatchJumpForward(ml::i64 target_index);
    void PatchJumpBackward(ml::i64 target_index);
    void PatchJumpForwardConditional(ml::i64 target_index);
    void PatchJumpBackwardConditional(ml::i64 target_index);
    Register CalcJump(ml::i64 target_index, bool is_forward, bool is_conditional) const;

    Register AllocateRegister();
    void FreeRegister(Register reg);
    void FreeRegisters(std::initializer_list<Register> regs);
    void FreeRegisters(const std::vector<Register>& regs);

    bool RegisterIsOwned(Register reg);

    Register PopRegBuffer();
    void ClearRegBuffer();

    void EnterScope();
    void ExitScope();

    void EnterLoop();
    void ExitLoop();

    void AddSymbol(std::string_view name, Register index);
    void RemoveSymbol(std::string_view name);

    LoopContext& CurrentLoop();

    struct RangeLoopRegisters {
        Register end, step, counter;
    };

    RangeLoopRegisters PerformRangeLoopSetup(const ast::LoopRange& node);

    void HandlePendingSkips();
    void HandlePendingBreaks();

    void HandleLoopControl(bool is_break, const ast::NodePtr& condition);
    void HandleDeclaration(const ast::Initializer& node, bool is_mutable);

    template <mv::ValuePrimitive VP>
    void CreateLiteral(const ast::Literal<VP>& literal) {
        const auto index = bytecode.AddConstant(literal.Get());
        if (constants.contains(index)) {
            reg_buffer.push_back(constants[index].register_index);
            return;
        }
        const auto reg = AllocateRegister();
        bytecode.Write(mv::Op::LoadConstant, {reg, index});

        constants[index] = {reg, scope_depth};

        reg_buffer.push_back(reg);
    }
};
} // namespace circe
