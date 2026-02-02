#include <sigil/ast/semantic-analyzer.hpp>
#include <sigil/ast/keywords.hpp>
#include <sigil/ast/syntax-tree.hpp>

#include <ranges>

namespace sigil {
using namespace mana::literals;
using namespace ast;
using enum PrimitiveType;

constexpr auto TB_ERROR    = "_TYPEBUFFER_ERROR_";

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
    : issue_counter {0},
      current_scope {GLOBAL_SCOPE} {
    RegisterPrimitives();
}

ml::i32 SemanticAnalyzer::IssueCount() const {
    return issue_counter;
}

const SymbolTable& SemanticAnalyzer::Globals() const {
    return globals;
}

const TypeTable& SemanticAnalyzer::Types() const {
    return types;
}

void SemanticAnalyzer::EnterScope() {
    ++current_scope;
}

void SemanticAnalyzer::ExitScope() {
    if (current_scope == GLOBAL_SCOPE) {
        Log->error("Internal Compiler Error: Attempted to exit global scope");
        return;
    }
    auto& symbols = CurrentFunction().locals;

    std::vector<std::string_view> to_remove;
    to_remove.reserve(symbols.size());

    for (const auto& [name, symbol] : symbols) {
        if (symbol.scope == current_scope) {
            to_remove.push_back(name);
        }
    }

    for (const auto& name : to_remove) {
        symbols.erase(name);
    }

    if (current_scope == CurrentFunction().scope) {
        function_stack.pop_back();
    }
    --current_scope;
}

void SemanticAnalyzer::Visit(const Artifact& artifact) {
    for (const auto& declaration : artifact.GetChildren()) {
        declaration->Accept(*this);
    }

    const bool lacks_main = std::ranges::none_of(GetFnTable(),
                                                 [](const auto& kv) {
                                                     return kv.first == ENTRY_POINT;
                                                 }
    );

    if (lacks_main) {
        Log->error("Program must contain an entry point function (Main)");
        ++issue_counter;
    }
}

// EnterScope should be called before this visiting this node,
// as certain things may fall outside of a scope body but still belong to it
// such as function parameters or loop bindings
void SemanticAnalyzer::Visit(const Scope& node) {
    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);
    }
    ExitScope();
}


void SemanticAnalyzer::Visit(const FunctionDeclaration& node) {
    const auto function_name = node.GetName();
    const auto return_type   = node.GetReturnType().empty()
                                 ? PrimitiveName(None)
                                 : node.GetReturnType();

    auto& functions = GetFnTable();
    if (functions.contains(function_name)) {
        Log->error("Redefinition of function '{}'", function_name);
        ++issue_counter;
    } else {
        functions[function_name].return_type = return_type;
    }

    const auto& params = node.GetParameters();
    if (IsEntryPoint(function_name)) {
        if (not params.empty()) {
            Log->error("Entry point function cannot have parameters");
            ++issue_counter;
        }

        if (return_type != PrimitiveName(None)) {
            Log->error("Entry point function cannot have a return type");
            ++issue_counter;
        }
    }

    auto& function = EnterFunction(function_name);
    for (const auto& param : params) {
        if (param.type.empty()) {
            Log->error("Parameter '{}' has no type annotation", param.name);
            ++issue_counter;
        }

        function.locals[param.name] = {
            param.type.empty() ? PrimitiveName(None) : param.type,
            function.scope,
            Mutability::Immutable
        };
    }

    node.GetBody()->Accept(*this);
}

void SemanticAnalyzer::Visit(const MutableDataDeclaration& node) {
    HandleInitializer(node, true);
}

void SemanticAnalyzer::Visit(const DataDeclaration& node) {
    HandleInitializer(node, false);
}

void SemanticAnalyzer::Visit(const Identifier& node) {
    const auto* symbol = GetSymbol(node.GetName());

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
        Log->error("Attempt to assign to undefined name '{}'", identifier);
        ++issue_counter;
    } else if (symbol->mutability != Mutability::Mutable) {
        Log->error("Attempt to assign to immutable binding '{}'", identifier);
        ++issue_counter;
    }

    node.GetValue()->Accept(*this);

    const auto expr_type = PopTypeBuffer();
    if (symbol != nullptr && not TypesMatch(expr_type, symbol->type)) {
        Log->error("Type mismatch: expected '{}', got '{}'", symbol->type, expr_type);
        ++issue_counter;
    }
}

void SemanticAnalyzer::Visit(const Return& node) {
    node.GetExpression()->Accept(*this);

    const auto type = PopTypeBuffer();
    if (not TypesMatch(CurrentFunction().return_type, type)) {
        Log->error("Type mismatch: Attempted to return '{1}' out of function with return type '{0}'",
                   CurrentFunction().return_type,
                   type
        );
        ++issue_counter;
    }
}

void SemanticAnalyzer::Visit(const Invocation& node) {
    const auto& functions = GetFnTable();
    const auto name       = node.GetIdentifier();

    if (not functions.contains(name)) {
        Log->error("Undefined identifier: No invocator exists with name '{}'", name);
        ++issue_counter;
    }

    for (const auto& arg : node.GetArguments()) {
        arg->Accept(*this);
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

void SemanticAnalyzer::RegisterPrimitives() {
    // register primitives
    types[PrimitiveName(I8)]    = TypeInfo {TypeSize::Byte};
    types[PrimitiveName(I16)]   = TypeInfo {TypeSize::Word};
    types[PrimitiveName(I32)]   = TypeInfo {TypeSize::DoubleWord};
    types[PrimitiveName(I64)]   = TypeInfo {TypeSize::QuadWord};
    types[PrimitiveName(Isize)] = TypeInfo {TypeSize::QuadWord};
    // we're not supporting 32bit systems for the foreseeable future
    // so isize/usize can just be 64-bit until maybe console support or something changes that

    types[PrimitiveName(U8)]    = TypeInfo {TypeSize::Byte};
    types[PrimitiveName(U16)]   = TypeInfo {TypeSize::Word};
    types[PrimitiveName(U32)]   = TypeInfo {TypeSize::DoubleWord};
    types[PrimitiveName(U64)]   = TypeInfo {TypeSize::QuadWord};
    types[PrimitiveName(Usize)] = TypeInfo {TypeSize::QuadWord};

    types[PrimitiveName(F32)] = TypeInfo {TypeSize::DoubleWord};
    types[PrimitiveName(F64)] = TypeInfo {TypeSize::QuadWord};

    types[PrimitiveName(Char)]   = TypeInfo {TypeSize::Byte};
    types[PrimitiveName(String)] = TypeInfo {TypeSize::Arbitrary};

    types[PrimitiveName(Byte)] = TypeInfo {TypeSize::Byte};
    types[PrimitiveName(Bool)] = TypeInfo {TypeSize::Byte};

    types[PrimitiveName(Fn)]   = TypeInfo {TypeSize::QuadWord}; // same as ptr
    types[PrimitiveName(None)] = TypeInfo {TypeSize::None};
}

FunctionTable& SemanticAnalyzer::GetFnTable() {
    return types[PrimitiveName(Fn)].functions;
}

const FunctionTable& SemanticAnalyzer::GetFnTable() const {
    return types.at(PrimitiveName(Fn)).functions;
}

Function& SemanticAnalyzer::EnterFunction(std::string_view name) {
    function_stack.push_back(name);
    auto& new_fn = GetFnTable()[name];

    EnterScope();

    new_fn.scope = current_scope;
    return new_fn;
}

std::string_view SemanticAnalyzer::CurrentFunctionName() const {
    return function_stack.back();
}

Function& SemanticAnalyzer::CurrentFunction() {
    return GetFnTable()[CurrentFunctionName()];
}

const Function& SemanticAnalyzer::CurrentFunction() const {
    return GetFnTable().at(CurrentFunctionName());
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
    const auto redef_error = [this](const std::string_view n) {
        Log->error("Redefinition of '{}'", n);
        ++issue_counter;
    };

    if (globals.contains(name)) {
        redef_error(name);
        return;
    }

    // we don't have constants yet
    const auto mutability = is_mutable ? Mutability::Mutable : Mutability::Immutable;

    if (current_scope == GLOBAL_SCOPE) {
        globals[name] = {type, current_scope, mutability};
        return;
    }

    auto& locals = CurrentFunction().locals;
    if (locals.contains(name)) {
        redef_error(name);
        return;
    }

    locals[name] = {type, current_scope, mutability};
}

const Symbol* SemanticAnalyzer::GetSymbol(std::string_view name) const {
    if (globals.contains(name)) {
        return &globals.at(name);
    }

    if (current_scope != GLOBAL_SCOPE) {
        const auto& locals = CurrentFunction().locals;
        if (locals.contains(name)) {
            return &locals.at(name);
        }
    }
    return nullptr;
}

void SemanticAnalyzer::HandleInitializer(const Initializer& node, const bool is_mutable) {
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

void SemanticAnalyzer::HandleRangedLoop(const LoopRange& node, const bool is_mutable) {
    ++loop_depth;

    EnterScope();

    // counter is mandatory in ranged loop
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

    node.GetBody()->Accept(*this);

    --loop_depth;
}
} // namespace sigil
