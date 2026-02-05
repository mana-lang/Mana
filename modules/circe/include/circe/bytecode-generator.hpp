#pragma once

#include <circe/register.hpp>

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

class BytecodeGenerator final : public ast::Visitor {
    using ScopeID = u8;

    struct Symbol {
        Register register_index;
        ScopeID scope;
    };

    struct Constant {
        Register register_index;
        ScopeID scope;
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
        i64 address = -1;
        RegisterFrame registers;
    };

    struct Call {
        std::string_view function;
        i64 instruction_index;
    };

    using ConstantTable = emhash8::HashMap<u16, Constant>;
    using SymbolTable   = emhash8::HashMap<std::string_view, Symbol>;
    using FunctionTable = emhash8::HashMap<std::string_view, Function>;

    ScopeID scope;

    SymbolTable symbols;
    ConstantTable constants;
    FunctionTable functions;

    RegisterFrame global_registers;
    std::vector<Register> register_buffer;

    std::vector<LoopContext> loop_stack;
    std::vector<std::string_view> function_stack;
    emhash8::HashMap<i64, std::string_view> pending_calls;

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

    void Visit(const ast::Return& node) override;
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

    CIRCE_NODISCARD RegisterFrame& Registers();

    Register PopRegBuffer();

    const Function& CurrentFunction() const;
    Function& CurrentFunction();
    std::string_view CurrentFunctionName() const;

    void ReturnNone();

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
    void HandleInitializer(const ast::Initializer& node, bool is_mutable);

    template <hexe::ValuePrimitive VP>
    void CreateLiteral(const ast::Literal<VP>& literal) {
        const auto index = bytecode.AddConstant(literal.Get());
        if (constants.contains(index)) {
            register_buffer.push_back(constants[index].register_index);
            return;
        }

        const auto reg = Registers().Allocate();
        bytecode.Write(hexe::Op::LoadConstant, {reg, index});

        constants[index] = {reg, scope};

        Registers().Lock(reg);
        register_buffer.push_back(reg);
    }
};
} // namespace circe
