#pragma once

#include <sigil/ast/syntax-tree.hpp>
#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>
#include <hexe/bytecode.hpp>

#include <emhash/emhash8.hpp>


namespace sigil {
class SemanticAnalyzer;
}

namespace circe {
using namespace mana::literals;
namespace ast = sigil::ast;

using Register = u16;

class BytecodeGenerator final : public ast::Visitor {
    using ScopeDepth = u8;

    struct Symbol {
        Register register_index;
        ScopeDepth scope_depth;
    };

    struct Constant {
        Register register_index;
        ScopeDepth scope_depth;
    };

    struct JumpInstruction {
        i64 jump_index;
        bool is_conditional;
    };

    struct LoopContext {
        std::vector<JumpInstruction> pending_breaks;
        std::vector<JumpInstruction> pending_skips;
    };

    struct Function {
        std::string_view return_type;
        u32 address = -1;
    };

    using SymbolTable   = emhash8::HashMap<std::string_view, Symbol>;
    using ConstantTable = emhash8::HashMap<u16, Constant>;
    using FunctionTable = emhash8::HashMap<std::string_view, Function>;

    SymbolTable symbols;
    ConstantTable constants;
    FunctionTable functions;

    u16 total_registers;
    ScopeDepth scope_depth;

    std::vector<Register> reg_buffer;
    std::vector<Register> free_regs;

    std::vector<LoopContext> loop_stack;
    std::vector<std::string_view> function_stack;

    hexe::ByteCode bytecode;

public:
    BytecodeGenerator();

    CIRCE_NODISCARD hexe::ByteCode Bytecode() const;

    void ObtainSemanticAnalysisInfo(const sigil::SemanticAnalyzer& analyzer);

    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::Scope& node) override;

    void Visit(const ast::FunctionDeclaration& node) override;
    void Visit(const ast::MutableDataDeclaration& node) override;
    void Visit(const ast::DataDeclaration& node) override;

    void Visit(const ast::Identifier& node) override;
    void Visit(const ast::Assignment& node) override;

    void Visit(const ast::Return& node) override {}
    void Visit(const ast::Invocation& node) override;

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

    void Visit(const ast::Literal<f64>& literal) override;
    void Visit(const ast::Literal<i64>& literal) override;
    void Visit(const ast::Literal<void>& node) override;
    void Visit(const ast::Literal<bool>& literal) override;

private:
    bool IsConditionalJumpOp(hexe::Op op) const;
    void JumpBackwards(i64 target_index);
    void JumpForward(i64 target_index);
    void JumpBackwardsConditional(hexe::Op op, Register condition_register, i64 target_index);
    void PatchJumpForward(i64 target_index);
    void PatchJumpBackward(i64 target_index);
    void PatchJumpForwardConditional(i64 target_index);
    void PatchJumpBackwardConditional(i64 target_index);
    Register CalcJump(i64 target_index, bool is_forward, bool is_conditional) const;

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
    void HandleDataBinding(const ast::Initializer& node, bool is_mutable);

    template <hexe::ValuePrimitive VP>
    void CreateLiteral(const ast::Literal<VP>& literal) {
        const auto index = bytecode.AddConstant(literal.Get());
        if (constants.contains(index)) {
            reg_buffer.push_back(constants[index].register_index);
            return;
        }

        const auto reg = AllocateRegister();
        bytecode.Write(hexe::Op::LoadConstant, {reg, index});

        constants[index] = {reg, scope_depth};

        reg_buffer.push_back(reg);
    }
};
} // namespace circe
