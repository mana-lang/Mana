#include <sigil/ast/semantic-analyzer.hpp>
#include <sigil/ast/keywords.hpp>
#include <sigil/ast/syntax-tree.hpp>

#include <ranges>

namespace sigil {
using namespace mana::literals;
using namespace ast;
using enum PrimitiveType;

constexpr auto TB_ERROR = "_TYPEBUFFER_ERROR_";

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
    RegisterBuiltins();
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

void SemanticAnalyzer::Visit(const Artifact& artifact) {
    RecordFunctionDeclarations(artifact);

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

void SemanticAnalyzer::Visit(const Scope& node) {
    ++current_scope;

    for (const auto& statement : node.GetStatements()) {
        statement->Accept(*this);
    }

    if (current_scope == CurrentFunction().scope) {
        function_stack.pop_back();
    }

    if (current_scope == GLOBAL_SCOPE) {
        Log->error("Internal Compiler Error: Attempted to exit global scope");
        return;
    }
    --current_scope;
}


void SemanticAnalyzer::Visit(const FunctionDeclaration& node) {
    const auto name = node.GetName();
    function_stack.push_back(name);

    auto& function = GetFnTable()[name];
    function.scope = current_scope + 1;

    for (const auto& param : node.GetParameters()) {
        auto& local = function.locals[param.name];

        local.scope      = function.scope;
        local.mutability = Mutability::Immutable;
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
        Log->error("Assignment type mismatch: expected '{}', got '{}'", symbol->type, expr_type);
        ++issue_counter;
    }
    PreventAssignmentWithNone(expr_type);
}

void SemanticAnalyzer::Visit(const Return& node) {
    node.GetExpression()->Accept(*this);
    const auto type = PopTypeBuffer();

    if (not TypesMatch(CurrentFunction().return_type, type)) {
        Log->error("Return type mismatch: Attempted to return '{1}' out of function with return type '{0}'",
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
        return;
    }

    const auto& fn   = functions.at(name);
    const auto& args = node.GetArguments();

    if (fn.param_count != args.size()) {
        Log->error("Function '{}' expects {} arguments, but {} were provided", name, fn.param_count, args.size());
        ++issue_counter;

        // still wanna buffer the function's type so errors don't propagate to silly places
        BufferType(fn.return_type);
        return;
    }

    // params are recorded in reverse order to resolve their types in comma sequences e.g. 'a, b: T'
    i64 i = 0;
    for (const auto& local : fn.locals | std::views::values) {
        if (not local.is_param) {
            continue;
        }

        args[i]->Accept(*this);
        ++i;

        const auto arg_type = PopTypeBuffer();
        if (not TypesMatch(arg_type, local.type)) {
            Log->error("Argument type mismatch: expected '{}', got '{}'", local.type, arg_type);
            ++issue_counter;
        }
    }

    BufferType(fn.return_type);
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

void SemanticAnalyzer::Visit(const Literal<bool>&) {
    BufferType(PrimitiveName(Bool));
}

void SemanticAnalyzer::Visit(const StringLiteral& string) {
    BufferType(PrimitiveName(String));
}

void SemanticAnalyzer::RecordFunctionDeclarations(const Artifact& artifact) {
    // not the biggest fan of dynamic_cast, but a whole other visitor just to collect
    // function declarations would be some otherworldly level of premature optimization
    for (const auto& declaration : artifact.GetChildren()) {
        if (const auto* fn_decl = dynamic_cast<const FunctionDeclaration*>(declaration.get())) {
            const auto name = fn_decl->GetName();

            auto& functions = GetFnTable();
            if (functions.contains(name)) {
                Log->error("Redefinition of function '{}'", name);
                ++issue_counter;
                continue;
            }

            auto& fn           = functions[name];
            const auto& params = fn_decl->GetParameters();
            fn.return_type     = fn_decl->GetReturnType();

            if (IsEntryPoint(name)) {
                if (not params.empty()) {
                    Log->error("Entry point function cannot have parameters");
                    ++issue_counter;
                }

                if (fn.return_type != PrimitiveName(None)) {
                    Log->error("Entry point function cannot have a return type");
                    ++issue_counter;
                }
            }

            // handle param types only, for invocation arity checks
            for (const auto& param : params) {
                if (param.type.empty()) {
                    Log->error("Parameter '{}' has no type annotation", param.name);
                    ++issue_counter;
                }

                fn.locals[param.name] = {param.type, true};
                ++fn.param_count;
            }
        }
    }
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

void SemanticAnalyzer::RegisterBuiltins() {
    auto& print         = GetFnTable()["Print"];
    print.return_type   = PrimitiveName(None);
    print.param_count   = 1;
    print.locals["str"] = {PrimitiveName(String), true};

    auto& printv         = GetFnTable()["PrintV"];
    printv.return_type   = PrimitiveName(None);
    printv.param_count   = 2;
    printv.locals["str"] = {PrimitiveName(String), true};
}

FunctionTable& SemanticAnalyzer::GetFnTable() {
    return types[PrimitiveName(Fn)].functions;
}

const FunctionTable& SemanticAnalyzer::GetFnTable() const {
    return types.at(PrimitiveName(Fn)).functions;
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

void SemanticAnalyzer::BufferType(const std::string_view type_name) {
    type_buffer[1] = type_name;
}

bool SemanticAnalyzer::TypesMatch(const std::string_view lhs, const std::string_view rhs) const {
    return lhs == rhs
           || (IsSignedIntegral(lhs) && IsSignedIntegral(rhs))
           || (IsUnsignedIntegral(lhs) && IsUnsignedIntegral(rhs))
           || (IsFloatPrimitive(lhs) && IsFloatPrimitive(rhs));
}

void SemanticAnalyzer::AddSymbol(const std::string_view name,
                                 const std::string_view type,
                                 const bool is_mutable,
                                 const ScopeID scope
) {
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

    if (scope == GLOBAL_SCOPE) {
        globals[name] = {type, scope, mutability};
        return;
    }

    auto& locals = CurrentFunction().locals;
    if (locals.contains(name)) {
        redef_error(name);
        return;
    }

    locals[name] = {type, scope, mutability};
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
        Log->error("Initializer: Type mismatch: expected '{}', got '{}'", annotation_type, initializer_type);
        ++issue_counter;
    }

    PreventAssignmentWithNone(initializer_type);

    AddSymbol(node.GetName(), annotation_type, is_mutable, current_scope);
}

// temporary, until we can elide every binding containing 'none'
void SemanticAnalyzer::PreventAssignmentWithNone(const std::string_view type) {
    if (type == PrimitiveName(None)) {
        Log->error("Cannot initialize binding of type '{}'. "
                   "This feature is planned for future versions of Mana.",
                   type
        );
        ++issue_counter;
    }
}

void SemanticAnalyzer::HandleRangedLoop(const LoopRange& node, const bool is_mutable) {
    ++loop_depth;

    // counter is mandatory in ranged loop
    AddSymbol(node.GetCounterName(), PrimitiveName(I64), is_mutable, current_scope + 1);

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
