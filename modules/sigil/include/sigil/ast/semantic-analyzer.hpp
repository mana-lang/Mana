#pragma once

#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

#include <emhash/emhash8.hpp>

#include <string_view>
#include <string>
#include <vector>

namespace sigil {
namespace ml = mana::literals;

constexpr auto GLOBAL_SCOPE = 0;

class SemanticAnalyzer final : public ast::Visitor {
    // in bits
    enum class TypeSize : ml::u8 {
        None       = 0,
        Byte       = 8,
        Word       = 16,
        DoubleWord = 32,
        QuadWord   = 64,

        Arbitrary = 0xFF
    };

    enum class Mutability : ml::u8 {
        Immutable,
        Mutable,
        Const,
    };

    struct Symbol {
        std::string_view type;
        ml::u8 scope          = GLOBAL_SCOPE;
        Mutability mutability = Mutability::Const;

        Symbol(std::string_view type, ml::u8 scope, Mutability mutability)
            : type {type},
              scope {scope},
              mutability {mutability} {}

        Symbol() = default;
    };

    using SymbolTable = emhash8::HashMap<std::string_view, Symbol>;

    struct Function {
        SymbolTable locals;
        std::string_view return_type;
        ml::u8 scope = GLOBAL_SCOPE;
    };

    using FunctionTable = emhash8::HashMap<std::string_view, Function>;

    struct TypeInfo {
        FunctionTable functions;

        TypeSize size = TypeSize::None;

        explicit TypeInfo(TypeSize size)
            : size {size} {}

        TypeInfo() = default;
    };

    using TypeTable = emhash8::HashMap<std::string_view, TypeInfo>;

    SymbolTable globals;
    TypeTable types;

    std::vector<std::string_view> function_stack;

    ml::i32 issue_counter;

    ml::u8 current_scope;
    ml::u8 loop_depth;

    // this gives the analyzer some awareness of the most recently resolved type
    // the buffer only holds one type at a time
    // reading from it expires the type
    std::array<std::string_view, 2> type_buffer;

public:
    SemanticAnalyzer();

    SIGIL_NODISCARD ml::i32 IssueCount() const;

    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::Scope& node) override;

    void Visit(const ast::FunctionDeclaration& node) override;
    void Visit(const ast::MutableDataDeclaration& node) override;
    void Visit(const ast::DataDeclaration& node) override;

    void Visit(const ast::Identifier& node) override;
    void Visit(const ast::Assignment& node) override;

    void Visit(const ast::Return& node) override;

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
    void RegisterPrimitives();
    FunctionTable& GetFnTable();
    const FunctionTable& GetFnTable() const;

    Function& EnterFunction(std::string_view name);
    std::string_view CurrentFunctionName() const;

    Function& CurrentFunction();
    const Function& CurrentFunction() const;

    void EnterScope();
    void ExitScope();

    std::string_view PopTypeBuffer();
    void BufferType(std::string_view type_name);

    bool TypesMatch(std::string_view lhs, std::string_view rhs) const;

    void AddSymbol(std::string_view name, std::string_view type, bool is_mutable);
    const Symbol* GetSymbol(std::string_view name) const;

    void HandleInitializer(const ast::Initializer& node, bool is_mutable);

    void HandleRangedLoop(const ast::LoopRange& node, bool is_mutable);
};
} // namespace sigil
