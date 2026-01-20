#pragma once

#include <sigil/ast/visitor.hpp>

#include <mana/literals.hpp>

#include <unordered_map>
#include <string_view>
#include <array>

namespace sigil::ast {
namespace ml = mana::literals;

class SemanticAnalyzer final : public Visitor {
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

private:
    void EnterScope();
    void ExitScope();

    std::string_view PopTypeBuffer();
    void BufferType(std::string_view type_name);

    void AddSymbol(std::string_view name, std::string_view type, bool is_mutable);
    const Datum* GetSymbol(std::string_view name) const;

    void HandleDeclaration(const class Binding& node, bool is_mutable);
};
} // namespace sigil::ast
