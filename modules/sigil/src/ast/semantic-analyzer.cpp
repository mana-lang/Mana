#include <sigil/ast/semantic-analyzer.hpp>
#include <sigil/ast/keywords.hpp>
#include <sigil/ast/syntax-tree.hpp>

namespace sigil {
using namespace mana::literals;
using namespace ast;
using enum PrimitiveType;

bool IsSignedIntegral(std::string_view type) {
    return type == PrimitiveName(I8)
           || type == PrimitiveName(I16)
           || type == PrimitiveName(I32)
           || type == PrimitiveName(I64);
}

bool IsUnsignedIntegral(std::string_view type) {
    return type == PrimitiveName(U8)
           || type == PrimitiveName(U16)
           || type == PrimitiveName(U32)
           || type == PrimitiveName(U64);
}

bool IsFloatPrimitive(std::string_view type) {
    return type == PrimitiveName(F32) || type == PrimitiveName(F64);
}

bool IsIntegral(const std::string_view type) {
    return IsSignedIntegral(type) || IsUnsignedIntegral(type);
}

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
    // so isize/usize can just be 64-bit until maybe console support or something changes that

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

ml::i32 SemanticAnalyzer::IssueCount() const {
    return issue_counter;
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
    HandleInitializer(node, true);
}

void SemanticAnalyzer::Visit(const DataDeclaration& node) {
    HandleInitializer(node, false);
}

void SemanticAnalyzer::Visit(const Identifier& node) {
    const auto* symbol = GetSymbol(node.GetName());
//
    if (symbol == nullptr) {
        Log->error("Undefined identifier '{}'", node.GetName());
        ++issue_counter;
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
        ++issue_counter;
    } else if (not symbol->is_mutable) {
        Log->error("Attempt to assign to immutable datum '{}'", identifier);
        ++issue_counter;
    }

    node.GetValue()->Accept(*this);

    const auto expr_type = PopTypeBuffer();
    if (symbol != nullptr && not TypesMatch(expr_type, symbol->type)) {
        Log->error("Type mismatch: expected '{}', got '{}'", symbol->type, expr_type);
        ++issue_counter;
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

void SemanticAnalyzer::Visit(const LoopFixed& node) {
    ++loop_depth;

    node.GetCountTarget()->Accept(*this);

    const auto counter_type = PopTypeBuffer();
    if (not IsIntegral(counter_type)) {
        Log->error("Loop count must be of integral type");
        ++issue_counter;
    }

    node.GetBody()->Accept(*this);

    --loop_depth;
}

void SemanticAnalyzer::Visit(const LoopRange& node) {
    HandleRangedLoop(node, false);
}

// only codegen cares about the difference
void SemanticAnalyzer::Visit(const LoopRangeMutable& node) {
    HandleRangedLoop(node, true);
}

void SemanticAnalyzer::Visit(const Break& node) {
    if (loop_depth == 0) {
        Log->error("Break outside loop");
        ++issue_counter;
        return;
    }

    if (const auto& cond = node.GetCondition()) {
        cond->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const Skip& node) {
    if (loop_depth == 0) {
        Log->error("Skip outside loop");
        ++issue_counter;
        return;
    }

    if (const auto& cond = node.GetCondition()) {
        cond->Accept(*this);
    }
}

void SemanticAnalyzer::Visit(const UnaryExpr& node) {
    const auto op = node.GetOp();
    node.GetVal().Accept(*this);
    const auto val_type = PopTypeBuffer();

    if (op == "!" && val_type != PrimitiveName(Bool)) {
        Log->error("Attempted to negate non-boolean expression");
        ++issue_counter;
    }
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

std::string_view SemanticAnalyzer::PopTypeBuffer() {
    type_buffer[0] = type_buffer[1];
    type_buffer[1] = TB_ERROR;
    return type_buffer[0];
}

void SemanticAnalyzer::BufferType(std::string_view type_name) {
    type_buffer[1] = type_name;
}

bool SemanticAnalyzer::TypesMatch(const std::string_view lhs, const std::string_view rhs) const {
    return lhs == rhs
           || (IsSignedIntegral(lhs) && IsSignedIntegral(rhs))
           || (IsUnsignedIntegral(lhs) && IsUnsignedIntegral(rhs))
           || (IsFloatPrimitive(lhs) && IsFloatPrimitive(rhs));
}

void SemanticAnalyzer::AddSymbol(std::string_view name, std::string_view type, bool is_mutable) {
    if (symbols.contains(name)) {
        Log->error("Redefinition of '{}'", name);
        ++issue_counter;
        return;
    }

    symbols[name] = {type, scope_depth, is_mutable};
}

const SemanticAnalyzer::Datum* SemanticAnalyzer::GetSymbol(std::string_view name) const {
    const auto it = symbols.find(name);
    return it != symbols.end() ? &it->second : nullptr;
}

void SemanticAnalyzer::HandleInitializer(const Initializer& node, bool is_mutable) {
    // evaluate expr first
    const auto& init    = node.GetInitializer();
    const bool has_init = init != nullptr;

    if (has_init) {
        init->Accept(*this);
    }

    const auto initializer_type = has_init ? PopTypeBuffer() : PrimitiveName(None);
    const auto annotation_type  = node.HasTypeAnnotation() ? node.GetTypeName() : initializer_type;

    if (not types.contains(annotation_type)) {
        Log->error("Unknown type '{}'", annotation_type);
        ++issue_counter;
    }

    if (has_init && not TypesMatch(initializer_type, annotation_type)) {
        Log->error("Type mismatch: expected '{}', got '{}'", annotation_type, initializer_type);
        ++issue_counter;
    }

    AddSymbol(node.GetName(), annotation_type, is_mutable);
}

void SemanticAnalyzer::HandleRangedLoop(const LoopRange& node, bool is_mutable) {
    ++loop_depth;

    // counter is mandatory in ranged loop
    ++scope_depth;
    AddSymbol(node.GetCounterName(), PrimitiveName(I64), is_mutable);

    const bool has_origin = node.GetOrigin() != nullptr;
    if (has_origin) {
        node.GetOrigin()->Accept(*this);
    }

    const auto start_type = has_origin ? PopTypeBuffer() : PrimitiveName(I64);

    node.GetDestination()->Accept(*this);
    const auto end_type = PopTypeBuffer();

    if (not IsIntegral(start_type) || not IsIntegral(end_type)) {
        Log->error("Range loop requires integral start and end values");
        ++issue_counter;
    }

    if (not IsSignedIntegral(start_type) || not IsSignedIntegral(end_type)) {
        Log->warn("Using unsigned integers in ranges is bug-prone. Prefer signed integers instead");
    }

    --scope_depth; // the range segment is part of the loop's scope

    node.GetBody()->Accept(*this);

    --loop_depth;
}
} // namespace sigil
