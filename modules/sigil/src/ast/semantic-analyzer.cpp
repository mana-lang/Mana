#include <sigil/ast/semantic-analyzer.hpp>
#include <sigil/ast/keywords.hpp>
#include <sigil/ast/syntax-tree.hpp>

namespace sigil::ast {
using namespace mana::literals;
using enum PrimitiveType;

SemanticAnalyzer::SemanticAnalyzer()
    : scope_depth {0},
      loop_depth {0},
      issue_counter {0} {
    // register primitives

    types[PrimitiveName(I8)]    = {TypeSize::Byte, 0};
    types[PrimitiveName(I16)]   = {TypeSize::Word, 0};
    types[PrimitiveName(I32)]   = {TypeSize::DoubleWord, 0};
    types[PrimitiveName(I64)]   = {TypeSize::QuadWord, 0};
    types[PrimitiveName(Isize)] = {TypeSize::QuadWord, 0};
    // we're not supporting 32bit systems for the foreseeable future

    types[PrimitiveName(U8)]    = {TypeSize::Byte, 0};
    types[PrimitiveName(U16)]   = {TypeSize::Word, 0};
    types[PrimitiveName(U32)]   = {TypeSize::DoubleWord, 0};
    types[PrimitiveName(U64)]   = {TypeSize::QuadWord, 0};
    types[PrimitiveName(Usize)] = {TypeSize::QuadWord, 0};

    types[PrimitiveName(F32)] = {TypeSize::DoubleWord, 0};
    types[PrimitiveName(F64)] = {TypeSize::QuadWord, 0};

    types[PrimitiveName(Char)]   = {TypeSize::Byte, 0};
    types[PrimitiveName(String)] = {TypeSize::Arbitrary, 0};

    types[PrimitiveName(Byte)] = {TypeSize::Byte, 0};
    types[PrimitiveName(Bool)] = {TypeSize::Byte, 0};
    types[PrimitiveName(None)] = {TypeSize::None, 0};
}

void SemanticAnalyzer::EnterScope() {
    ++scope_depth;
}

void SemanticAnalyzer::ExitScope() {
    std::vector<std::string_view> to_remove;
    to_remove.reserve(symbols.size());

    for (const auto& [name, symbol] : symbols) {
        if (symbol.scope_depth == scope_depth) {
            to_remove.push_back(name);
        }
    }

    for (const auto& name : to_remove) {
        symbols.erase(name);
    }

    --scope_depth;
}

constexpr auto TB_ERROR = "_TYPEBUFFER_ERROR_";

std::string_view SemanticAnalyzer::PopTypeBuffer() {
    type_buffer[0] = type_buffer[1];
    type_buffer[1] = TB_ERROR;
    return type_buffer[0];
}

void SemanticAnalyzer::BufferType(std::string_view type_name) {
    type_buffer[1] = type_name;
}

void SemanticAnalyzer::AddSymbol(std::string_view name, std::string_view type, bool is_mutable) {
    if (symbols.contains(name)) {
        Log->error("Redefinition of '{}'", name);
        return;
    }

    symbols[name] = {type, scope_depth, is_mutable};
}

const SemanticAnalyzer::Datum* SemanticAnalyzer::GetSymbol(std::string_view name) const {
    const auto it = symbols.find(name);
    return it != symbols.end() ? &it->second : nullptr;
}

void SemanticAnalyzer::HandleDeclaration(const Binding& node, bool is_mutable) {
    // evaluate expr first
    const auto& init = node.GetInitializer();
    if (init != nullptr) {
        init->Accept(*this);
    }
    const auto expr_type = PopTypeBuffer();

    const auto type = node.HasTypeAnnotation() ? node.GetTypeName() : expr_type;

    if (not types.contains(type)) {
        Log->error("Unknown type '{}'", type);
        return;
    }

    // might have to also '&& node.HasTypeAnnotation()'
    if (type != expr_type) {
        Log->error("Type mismatch: expected '{}', got '{}'", type, expr_type);
        return;
    }

    AddSymbol(node.GetName(), type, is_mutable);
}

// Visit methods
void SemanticAnalyzer::Visit(const Artifact& artifact) {
    EnterScope();
    for (const auto& statement : artifact.GetChildren()) {
        statement->Accept(*this);
    }
    ExitScope();
}

void SemanticAnalyzer::Visit(const Scope& node) {
    EnterScope();
    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);
    }
    ExitScope();
}

void SemanticAnalyzer::Visit(const MutableDataDeclaration& node) {
    HandleDeclaration(node, true);
}

void SemanticAnalyzer::Visit(const DataDeclaration& node) {
    HandleDeclaration(node, false);
}

void SemanticAnalyzer::Visit(const Identifier& node) {
    const auto* symbol = GetSymbol(node.GetName());

    if (symbol == nullptr) {
        Log->error("Undefined identifier '{}'", node.GetName());
        return;
    }

    // queue this symbol's type for evaluation
    BufferType(symbol->type);
}

void SemanticAnalyzer::Visit(const Assignment& node) {
    const auto identifier = node.GetIdentifier();
    const auto* symbol    = GetSymbol(identifier);

    if (symbol == nullptr) {
        Log->error("Attempt to assign to undefined datum '{}'", identifier);
        return;
    }


    if (not symbol->is_mutable) {
        Log->error("Attempt to assign to immutable datum '{}'", identifier);
        return; // is it wise to return here and avoid error checking on the assigned expression?
    }

    node.GetValue()->Accept(*this);

    const auto expr_type = PopTypeBuffer();
    if (expr_type != symbol->type) {
        Log->error("Type mismatch: expected '{}', got '{}'", symbol->type, expr_type);
    }
}

void SemanticAnalyzer::Visit(const If& node) {
    node.GetCondition()->Accept(*this);
    node.GetThenBlock()->Accept(*this);

    if (const auto& else_branch = node.GetElseBranch()) {
        else_branch->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const Loop& node) {
    ++loop_depth;
    node.GetBody()->Accept(*this);
    --loop_depth;
}

void SemanticAnalyzer::Visit(const LoopIf& node) {
    ++loop_depth;
    node.GetCondition()->Accept(*this);
    node.GetBody()->Accept(*this);
    --loop_depth;
}

void SemanticAnalyzer::Visit(const LoopIfPost& node) {
    ++loop_depth;
    node.GetBody()->Accept(*this);
    node.GetCondition()->Accept(*this);
    --loop_depth;
}

void SemanticAnalyzer::Visit(const LoopRange& node) {
    ++loop_depth;

    // counter is mandatory in ranged loop and input is assumed to be correct
    AddSymbol(node.GetCounter(), PrimitiveName(I64), false);

    node.GetStart()->Accept(*this);
    node.GetEnd()->Accept(*this);
    node.GetBody()->Accept(*this);

    --loop_depth;
}

void SemanticAnalyzer::Visit(const LoopFixed& node) {
    ++loop_depth;

    if (node.HasCounter()) {
        AddSymbol(node.GetCounter(), PrimitiveName(I64), false);
    }

    node.GetLimit()->Accept(*this);
    node.GetBody()->Accept(*this);

    --loop_depth;
}

void SemanticAnalyzer::Visit(const Break& node) {
    if (loop_depth == 0) {
        Log->error("Break outside loop");
        return;
    }

    if (const auto& cond = node.GetCondition()) {
        cond->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const Skip& node) {
    if (loop_depth == 0) {
        Log->error("Skip outside loop");
        return;
    }

    if (const auto& cond = node.GetCondition()) {
        cond->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const UnaryExpr& node) {
    node.GetVal().Accept(*this);
}

void SemanticAnalyzer::Visit(const BinaryExpr& node) {
    node.GetRight().Accept(*this);
    node.GetLeft().Accept(*this);
}

void SemanticAnalyzer::Visit(const ArrayLiteral& array) {
    for (const auto& value : array.GetValues()) {
        value->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const Literal<f64>&) {
    BufferType(PrimitiveName(F64));
}

void SemanticAnalyzer::Visit(const Literal<i64>&) {
    BufferType(PrimitiveName(I64));
}

void SemanticAnalyzer::Visit(const Literal<void>&) {
    BufferType(PrimitiveName(None));
}

void SemanticAnalyzer::Visit(const Literal<bool>&) {
    BufferType(PrimitiveName(Bool));
}
} // namespace sigil::ast
