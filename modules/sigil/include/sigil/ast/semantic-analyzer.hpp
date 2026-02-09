#pragma once

#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

#include <emhash/emhash8.hpp>

#include <string_view>
#include <vector>

namespace sigil {
using namespace mana::literals;

// in bits
enum class TypeSize : u8 {
    None       = 0,
    Byte       = 8,
    Word       = 16,
    DoubleWord = 32,
    QuadWord   = 64,

    Arbitrary = 0xFF
};

enum class Mutability : u8 {
    Immutable,
    Mutable,
    Const,
};

using ScopeID = i8;

constexpr ScopeID GLOBAL_SCOPE = 0;

struct Symbol {
    std::string_view type;
    ScopeID scope         = GLOBAL_SCOPE;
    Mutability mutability = Mutability::Const;
    bool is_param         = false;

    Symbol(std::string_view type, ScopeID scope, Mutability mutability)
        : type {type},
          scope {scope},
          mutability {mutability} {}

    Symbol(std::string_view type, bool is_param)
        : type {type},
          scope {-1},
          is_param {is_param} {}

    Symbol() = default;
};

using SymbolTable = emhash8::HashMap<std::string_view, Symbol>;

struct Function {
    SymbolTable locals;
    std::string_view return_type;
    ScopeID scope  = GLOBAL_SCOPE;
    u8 param_count = 0;
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

class SemanticAnalyzer final : public ast::Visitor {
    SymbolTable globals;
    TypeTable types;

    std::vector<std::string_view> function_stack;

    // this gives the analyzer some awareness of the most recently resolved type
    // the buffer only holds one type at a time
    // reading from it expires the type
    std::array<std::string_view, 2> type_buffer;

    i32 issue_counter;

    ScopeID current_scope;
    u8 loop_depth;

public:
    SemanticAnalyzer();

    SIGIL_NODISCARD i32 IssueCount() const;

    SIGIL_NODISCARD const SymbolTable& Globals() const;
    SIGIL_NODISCARD const TypeTable& Types() const;

public:
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
    void Visit(const ast::Literal<bool>& literal) override;

    void Visit(const ast::StringLiteral& string) override;

private:
    void RecordFunctionDeclarations(const ast::Artifact& artifact);

    void RegisterPrimitives();
    void RegisterBuiltins();

    FunctionTable& GetFnTable();
    const FunctionTable& GetFnTable() const;

    std::string_view CurrentFunctionName() const;

    Function& CurrentFunction();
    const Function& CurrentFunction() const;

    std::string_view PopTypeBuffer();
    void BufferType(std::string_view type_name);

    bool TypesMatch(std::string_view lhs, std::string_view rhs) const;

    void AddSymbol(std::string_view name, std::string_view type, bool is_mutable, ScopeID scope);
    const Symbol* GetSymbol(std::string_view name) const;

    void HandleInitializer(const ast::Initializer& node, bool is_mutable);
    void PreventAssignmentWithNone(std::string_view type);

    void HandleRangedLoop(const ast::LoopRange& node, bool is_mutable);
};
} // namespace sigil
