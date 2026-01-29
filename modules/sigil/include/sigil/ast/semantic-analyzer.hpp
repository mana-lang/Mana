#pragma once

#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

#include <unordered_map>
#include <string_view>
#include <array>

namespace sigil {
//
namespace ml = mana::literals;

class SemanticAnalyzer final : public ast::Visitor {
    struct Datum {
        std::string_view type;
        ml::u8 scope_depth;
        bool is_mutable;
    };

    // in bits
    enum class TypeSize : ml::u8 {
        None       = 0,
        Byte       = 8,
        Word       = 16,
        DoubleWord = 32,
        QuadWord   = 64,

        Arbitrary = 0xFF
    };

    struct TypeInfo {
        TypeSize size;
        ml::u8 scope_depth;
    };

    using SymbolTable = std::unordered_map<std::string_view, Datum>;
    using TypeTable   = std::unordered_map<std::string_view, TypeInfo>;

    SymbolTable symbols;
    TypeTable types;

    ml::u8 scope_depth;
    ml::u8 loop_depth;

    ml::i32 issue_counter;

    // this gives the analyzer some awareness of the most recently resolved type
    // the buffer only holds one type at a time
    // reading from it expires the type
    std::array<std::string_view, 2> type_buffer;

public:
    SemanticAnalyzer();

    SIGIL_NODISCARD ml::i32 IssueCount() const;

    void Visit(const ast::Artifact& artifact) override;
    void Visit(const ast::Scope& node) override;

    void Visit(const ast::FunctionDeclaration& node) override {}
    void Visit(const ast::MutableDataDeclaration& node) override;
    void Visit(const ast::DataDeclaration& node) override;

    void Visit(const ast::Parameter& node) override {}
    void Visit(const ast::Identifier& node) override;
    void Visit(const ast::Assignment& node) override;

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
    void EnterScope();
    void ExitScope();

    std::string_view PopTypeBuffer();
    void BufferType(std::string_view type_name);

    bool TypesMatch(std::string_view lhs, std::string_view rhs) const;

    void AddSymbol(std::string_view name, std::string_view type, bool is_mutable);
    const Datum* GetSymbol(std::string_view name) const;

    void HandleInitializer(const ast::Initializer& node, bool is_mutable);

    void HandleRangedLoop(const ast::LoopRange& node, bool is_mutable);
};
} // namespace sigil
