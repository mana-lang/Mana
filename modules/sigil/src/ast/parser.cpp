#include <sigil/ast/parser.hpp>
#include <sigil/core/logger.hpp>
#include <sigil/ast/source-file.hpp>
#include <sigil/ast/syntax-tree.hpp>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <fstream>

namespace sigil {
using namespace ast;
using namespace mana::literals;

Parser::Parser(TokenStream&& tokens)
    : tokens {std::move(tokens)},
      cursor {},
      parse_tree {Rule::Undefined} {}

Parser::Parser(const TokenStream& tokens)
    : tokens {tokens},
      cursor {},
      parse_tree {Rule::Undefined} {}

Parser::Parser()
    : cursor {},
      parse_tree {Rule::Undefined} {}

void Parser::AcquireTokens(const TokenStream& tks) {
    tokens = tks;
}

void Parser::AcquireTokens(TokenStream&& tks) {
    tokens = std::move(tks);
}

bool Parser::Parse() {
    if (tokens.empty()) {
        Log->error("No tokens to parse");
        return false;
    }

    parse_tree.rule = Rule::Artifact;

    cursor = 0;
    while (ProgressedParseTree(parse_tree)) {}

    ConstructAST(parse_tree);


    // In case there's any trailing newlines
    SkipNewlines();

    return Expect(CurrentToken().type == TokenType::Eof,
                  parse_tree,
                  "Expected EOF"
    );
}

auto Parser::ViewParseTree() const -> const ParseNode& {
    return parse_tree;
}

auto Parser::ViewTokenStream() const -> const TokenStream& {
    return tokens;
}

auto Parser::AST() const -> Node* {
    return syntax_tree.get();
}

void Parser::PrintParseTree() const {
    Log->debug("Parse tree for artifact '{}'\n\n{}",
               Source().Name(),
               EmitParseTree(parse_tree)
    );
}

void Parser::EmitParseTree(const std::string_view file_name) const {
    std::ofstream out {std::string(file_name) + std::string(".ptree")};
    if (not out.is_open()) {
        Log->error("Failed to open file '{}' for writing", file_name);
        return;
    }

    out << EmitParseTree(parse_tree);
    if (out.fail()) {
        Log->error("Failed to write to file '{}'", file_name);
        return;
    }
    Log->info("Emitted parse tree to file '{}'", file_name);
}

std::string Parser::EmitParseTree() const {
    return EmitParseTree(parse_tree);
}

std::string Parser::EmitParseTree(const ParseNode& node, std::string prepend) const {
    std::string ret;
    if (node.rule == Rule::Artifact) {
        ret = fmt::format("[{}] -> {}\n\n",
                          magic_enum::enum_name(node.rule),
                          Source().Name()
        );
    } else {
        ret.append(fmt::format("{}[{}]\n", prepend, magic_enum::enum_name(node.rule)));

        prepend.append("== ");

        if (not node.tokens.empty()) {
            std::ranges::replace(prepend, '=', '-');

            for (const auto& token : node.tokens) {
                if (token.type == TokenType::Terminator) {
                    continue;
                }
                ret.append(fmt::format("{} [{}] -> {}\n",
                                       prepend,
                                       magic_enum::enum_name(token.type),
                                       FetchTokenText(token)
                    )
                );
            }

            std::ranges::replace(prepend, '-', '=');
        }
    }

    if (not node.branches.empty()) {
        for (auto& branch : node.branches) {
            ret.append(EmitParseTree(*branch, prepend));
        }
    }

    if (not node.branches.empty() && node.IsRoot()) {
        ret.append("\n");
    }

    return ret;
}

Token Parser::CurrentToken() const {
    return tokens[cursor];
}

Token Parser::PeekNextToken() const {
    return tokens[cursor + 1];
}

Token Parser::NextToken() {
    return tokens[++cursor];
}

Token Parser::GetAndCycleToken() {
    return tokens[cursor++];
}

bool Parser::SkipNewlines() {
    bool ret = false;

    while (cursor < tokens.size()
           && CurrentToken().type == TokenType::Terminator
           && FetchTokenText(CurrentToken()) != ";") {
        ret = true;
        ++cursor;
    }

    return ret;
}

void Parser::AddTokensTo(ParseNode& node, const TokenType delimiter) {
    while (CurrentToken().type != delimiter) {
        node.tokens.push_back(GetAndCycleToken());
    }

    AddCycledTokenTo(node);
}

void Parser::AddTokensTo(ParseNode& node, const i64 count) {
    for (i64 i = 0; i < count; ++i) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::AddCurrentTokenTo(ParseNode& node) const {
    if (cursor < tokens.size()) {
        node.tokens.push_back(CurrentToken());
    }
}

void Parser::AddCycledTokenTo(ParseNode& node) {
    if (cursor < tokens.size()) {
        node.tokens.push_back(GetAndCycleToken());
    }
}

void Parser::SkipCurrentToken() {
    ++cursor;
}

void Parser::SkipTokens(const ml::i32 count) {
    cursor += count;
}

bool Parser::ProgressedParseTree(ParseNode& node) {
    // don't process eof (final token) before quitting
    if (cursor + 1 >= tokens.size() - 1) {
        return false;
    }

    // we want to skip over empty lines
    if (CurrentToken().type == TokenType::Terminator) {
        SkipCurrentToken();
        return true;
    }

    return MatchedStatement(node);
}

void Parser::ConstructAST(const ParseNode& node) {
    if (node.rule != Rule::Artifact) {
        Log->error("Top-level p-tree node was not 'Artifact' but {}",
                   magic_enum::enum_name(node.rule)
        );
        return;
    }

    if (node.IsLeaf()) {
        Log->error("Empty module, no AST can be constructed");
        return;
    }

    syntax_tree = std::make_unique<Artifact>(Source().Name());

    PropagateStatements(node, syntax_tree.get());
}

bool Parser::Expect(const bool condition,
                    ParseNode& node,
                    const std::string_view error_message
) const {
    if (not condition) {
        Log->error("Line {} -> {}", CurrentToken().line, error_message);
        node.rule = Rule::Mistake;
        return false;
    }
    return true;
}

// stmt = fn_decl | if_stmt | loop | (ret_stmt | loop_ctl | decl | assign | expr) TERMINATOR
bool Parser::MatchedStatement(ParseNode& node) {
    auto& stmt = node.NewBranch(Rule::Statement);

    // block statements aren't terminated since they have a scope, so we exit early on match
    if (MatchedFunctionDeclaration(stmt)
        || MatchedIfBlock(stmt)
        || MatchedLoop(stmt)) {
        return true;
    }

    const bool is_statement = MatchedReturnStatement(stmt)
                              || MatchedLoopControl(stmt)
                              || MatchedDataDeclaration(stmt)
                              || MatchedAssignment(stmt)
                              || MatchedExpression(stmt);

    if (not is_statement) {
        if (stmt.branches.empty()) {
            // if stmt has branches, there may be a Rule::Mistake, so we only pop on dead match
            node.PopBranch();
        }
        return false;
    }

    if (not Expect(CurrentToken().type == TokenType::Terminator,
                   stmt,
                   "Expected terminator"
    )) {
        return true;
    }

    AddCycledTokenTo(stmt);
    return true;
}

bool Parser::MatchedScope(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_BraceLeft) {
        return false;
    }

    auto& scope = node.NewBranch(Rule::Scope);
    AddCycledTokenTo(scope);

    SkipNewlines();

    // exhaust all statements within the scope
    // no Expect() because empty scopes are valid
    while (MatchedStatement(scope)) {
        SkipNewlines();
    }

    SkipNewlines();

    if (Expect(CurrentToken().type == TokenType::Op_BraceRight, scope, "Expected closing brace '}' at end of scope")) {
        AddCycledTokenTo(scope);
    }

    return true;
}

// if_condition = KW_IF expr
bool Parser::MatchedIfCondition(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_if) {
        return false;
    }

    AddCycledTokenTo(node);

    Expect(MatchedExpression(node), node, "Expected expression");
    return true;
}

// if_block = KW_IF expr scope if_tail?
bool Parser::MatchedIfBlock(ParseNode& node) {
    auto& if_block = node.NewBranch(Rule::If);

    if (not MatchedIfCondition(if_block)) {
        node.PopBranch();
        return false;
    }

    if (not Expect(MatchedScope(if_block), if_block, "Expected scope for if-block")) {
        return true;
    }

    // since if-tails are optional, we don't care whether they match successfully
    MatchedIfTail(if_block);

    return true;
}

// if_tail = KW_ELSE (if_stmt | scope)
bool Parser::MatchedIfTail(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_else) {
        return false;
    }

    auto& if_tail = node.NewBranch(Rule::IfTail);
    AddCycledTokenTo(if_tail);

    // else if
    if (MatchedIfBlock(if_tail)) {
        return true;
    }

    // else
    Expect(MatchedScope(if_tail), if_tail, "Expected scope for else-block");

    return true;
}

// loop_control = (KW_SKIP | KW_BREAK) (OP_BINDING ID)? if_condition?
bool Parser::MatchedLoopControl(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_skip && CurrentToken().type != TokenType::KW_break) {
        return false;
    }

    auto& loop_control = node.NewBranch(Rule::LoopControl);
    AddCycledTokenTo(loop_control);

    // break/skip if cond
    MatchedIfCondition(loop_control);

    // break/skip (if cond)? => A
    if (CurrentToken().type == TokenType::Op_Binding) {
        AddCycledTokenTo(loop_control);

        if (not Expect(CurrentToken().type == TokenType::Identifier,
                       loop_control,
                       "Expected identifier after loop control keyword"
            )
        ) {
            return false;
        }
        AddCycledTokenTo(loop_control);
    }

    return true;
}

// loop = KW_LOOP (ID ':')? loop_body
bool Parser::MatchedLoop(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_loop) {
        return false;
    }

    auto& loop = node.NewBranch(Rule::Loop);
    SkipCurrentToken();

    if (CurrentToken().type == TokenType::Identifier
        && PeekNextToken().type == TokenType::Op_Colon
    ) {
        AddTokensTo(loop, TokenType::Op_Colon);
    }

    Expect(MatchedLoopBody(loop), loop, "Expected loop body");

    return true;
}

bool Parser::MatchedLoopBody(ParseNode& node) {
    // infinite/post-conditional
    // loop_body = scope (OP_BINDING if_condition)?
    if (MatchedScope(node)) {
        if (CurrentToken().type == TokenType::Op_Binding) {
            AddCycledTokenTo(node);
            node.rule = Rule::LoopIfPost;

            Expect(MatchedIfCondition(node), node, "Expected if condition after '=>'");
        }
        // don't need to do anything if loop is infinite
        return true;
    }

    // conditional
    // loop_body = if_condition scope
    if (MatchedIfCondition(node)) {
        node.rule = Rule::LoopIf;
        Expect(MatchedScope(node), node, "Expected scope for loop body");
        return true;
    }

    // fixed/ranged iteration
    // loop_body = expr (OP_RANGE expr)? (OP_BINDING mut? ID)? scope
    if (not MatchedExpression(node)) {
        // we've matched against nothing atp, so just exit early
        return false;
    }

    if (MatchedScope(node)) {
        // loop x {}
        node.rule = Rule::LoopFixed;

        Expect(node.branches[0]->rule != Rule::Unary
               && node.branches[0]->tokens[0].type != TokenType::Op_Minus,
               node,
               "Negative fixed loops lead to unexpected behaviour"
        );

        return true;
    }

    // we've matched an expr, but no scope.
    // next token must be either op_range or a op_binding
    if (CurrentToken().type != TokenType::Op_Range) {
        if (not Expect(CurrentToken().type == TokenType::Op_Binding, node, "Expected range or binding")) {
            return true;
        }
        SkipCurrentToken();

        if (CurrentToken().type == TokenType::KW_mut) {
            // loop x => mut
            AddCycledTokenTo(node);
        }

        if (not Expect(CurrentToken().type == TokenType::Identifier, node, "Range must bind to an identifier")) {
            return true;
        }

        // loop x => mut? y
        AddCycledTokenTo(node);
        node.rule = Rule::LoopRange;

        Expect(MatchedScope(node), node, "Expected scope for loop body");
        return true;
    }
    // it's a full range
    SkipCurrentToken();

    if (not Expect(MatchedExpression(node), node, "Range operator takes two operands")) {
        return true;
    }

    if (not Expect(CurrentToken().type == TokenType::Op_Binding,
                   node,
                   "Expected binding operator after range expression"
    )) {
        return true;
    }

    // loop x..y =>
    SkipCurrentToken();

    if (CurrentToken().type == TokenType::KW_mut) {
        // loop x..y => mut
        AddCycledTokenTo(node);
    }
    if (not Expect(CurrentToken().type == TokenType::Identifier, node, "Range must bind to identifier")) {
        return true;
    }

    // loop x..y => mut? z
    AddCycledTokenTo(node);
    node.rule = Rule::LoopRange;

    Expect(MatchedScope(node), node, "Expected scope for loop body");
    return true;
}

// fn_decl = KW_FN ID param_list ret_type? scope
bool Parser::MatchedFunctionDeclaration(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_fn) {
        return false;
    }
    SkipCurrentToken();

    auto& fn_decl = node.NewBranch(Rule::FunctionDeclaration);

    // fn a
    if (not Expect(CurrentToken().type == TokenType::Identifier, fn_decl, "Expected function name")) {
        return true;
    }
    AddCycledTokenTo(fn_decl);

    // fn a(b: i32, c, d: f64)
    if (not Expect(MatchedParameterList(fn_decl), fn_decl, "Expected parameter list")) {
        return true;
    }

    // fn a() -> i32
    if (CurrentToken().type == TokenType::Op_ReturnType) {
        SkipCurrentToken();

        Expect(PeekNextToken().type == TokenType::Identifier, fn_decl, "Expected return type");
        AddCycledTokenTo(fn_decl);
    }

    // fn a() {}
    Expect(MatchedScope(fn_decl), fn_decl, "Expected function body");
    return true;
}

// param_list = '(' (param (',' param)*)? ')'
bool Parser::MatchedParameterList(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_ParenLeft) {
        return false;
    }

    auto& param_list = node.NewBranch(Rule::ParameterList);

    // fn x()
    if (PeekNextToken().type == TokenType::Op_ParenRight) {
        SkipTokens(2);
        return true;
    }

    // param = ID (':' ID)?
    const auto handle_param = [&param_list, this]() {
        if (CurrentToken().type != TokenType::Identifier) {
            return false;
        }

        auto& param = param_list.NewBranch(Rule::Parameter);
        AddCycledTokenTo(param);
        if (CurrentToken().type != TokenType::Op_Colon) {
            return true;
        }
        SkipCurrentToken();

        if (not Expect(CurrentToken().type == TokenType::Identifier, param, "Expected type")) {
            return true;
        }
        AddCycledTokenTo(param);
        return true;
    };

    // handle first parameter
    if (not Expect(handle_param(), param_list, "Expected parameter")) {
        return true;
    }

    // handle rest
    while (CurrentToken().type == TokenType::Op_Comma) {
        SkipCurrentToken();
        if (not Expect(handle_param(), param_list, "Expected parameter")) {
            return true;
        }
    }

    Expect(CurrentToken().type == TokenType::Op_ParenRight, param_list, "Expected closing parenthesis");

    return true;
}

// ret_stmt = KW_RETURN expr?
bool Parser::MatchedReturnStatement(ParseNode& node) {
    if (CurrentToken().type != TokenType::KW_return) {
        return false;
    }
    SkipCurrentToken();

    auto& ret_stmt = node.NewBranch(Rule::ReturnStatement);

    MatchedExpression(ret_stmt);

    return true;
}

bool IsPrimitiveKeyword(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case KW_i8:
    case KW_i16:
    case KW_i32:
    case KW_i64:
    case KW_u8:
    case KW_u16:
    case KW_u32:
    case KW_u64:
    case KW_f32:
    case KW_f64:
    case KW_byte:
    case KW_bool:
    case KW_char:
    case KW_string:
    case KW_isize:
    case KW_usize:
    case Lit_none: // 'none' is a literal as well as a type
        return true;
    default:
        return false;
    }
}


// data_decl = KW_MUT? KW_DATA ID (':' ID)? ('=' expr)?
bool Parser::MatchedDataDeclaration(ParseNode& node) {
    const bool matched_keywords = CurrentToken().type == TokenType::KW_data
                                  || (CurrentToken().type == TokenType::KW_mut
                                      && PeekNextToken().type == TokenType::KW_data);
    if (not matched_keywords) {
        return false;
    }

    auto& decl = node.NewBranch(CurrentToken().type == TokenType::KW_mut
                                    ? Rule::MutableDataDeclaration
                                    : Rule::DataDeclaration
    );

    // covers both 'data x' and 'mut data x'
    AddTokensTo(decl, TokenType::KW_data);
    if (not Expect(CurrentToken().type == TokenType::Identifier,
                   decl,
                   "Expected identifier"
    )) {
        return true;
    }
    AddCycledTokenTo(decl);

    // data x: i32
    bool is_annotated = CurrentToken().type == TokenType::Op_Colon;
    if (is_annotated) {
        AddCycledTokenTo(decl);

        if (Expect(IsPrimitiveKeyword(CurrentToken().type) || CurrentToken().type == TokenType::Identifier,
                   decl,
                   "Expected type"
        )) {
            AddCycledTokenTo(decl);
        }
    }

    // data x: i32;
    if (CurrentToken().type == TokenType::Terminator) {
        // (mut) data x; -> error
        Expect(is_annotated, decl, "Expected annotation for uninitialized type");
        return true;
    }

    if (not Expect(CurrentToken().type == TokenType::Op_Assign,
                   decl,
                   "Expected '='"
    )) {
        return true;
    }
    AddCycledTokenTo(decl);

    Expect(MatchedExpression(decl),
           decl,
           "Expected expression"
    );

    return true;
}

bool IsCompoundAssignment(const TokenType op) {
    return op == TokenType::Op_AddAssign
           || op == TokenType::Op_SubAssign
           || op == TokenType::Op_MulAssign
           || op == TokenType::Op_DivAssign
           || op == TokenType::Op_ModAssign;
}

bool Parser::MatchedAssignment(ParseNode& node) {
    if (CurrentToken().type != TokenType::Identifier) {
        return false;
    }

    const TokenType op = PeekNextToken().type;
    if (op != TokenType::Op_Assign && not IsCompoundAssignment(op)) {
        return false;
    }

    auto& assignment = node.NewBranch(Rule::Assignment);
    AddTokensTo(assignment, op);

    Expect(MatchedExpression(assignment), assignment, "Expected expression");
    return true;
}

// elem_list = expr (',' expr)* (',')?  ;
bool Parser::MatchedElemList(ParseNode& node) {
    using enum TokenType;

    auto& elem_list {node.NewBranch()};
    elem_list.rule = Rule::ElemList;

    SkipNewlines();

    if (not MatchedExpression(elem_list)) {
        node.PopBranch();
        return false;
    }

    // trailing comma allowed
    while (CurrentToken().type == Op_Comma) {
        AddCycledTokenTo(elem_list); // ','

        SkipNewlines();

        if (not MatchedExpression(elem_list)) {
            break;
        }

        SkipNewlines();
    }

    return true;
}

// array_literal = '[' elem_list? ']'   ;
bool Parser::MatchedArrayLiteral(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_BracketLeft) {
        return false;
    }
    auto& array_literal {node.NewBranch()};
    array_literal.rule = Rule::ArrayLiteral;
    AddCycledTokenTo(array_literal); // '['

    // Allow [\n] etc.
    SkipNewlines();

    // []
    if (CurrentToken().type == TokenType::Op_BracketRight) {
        AddCycledTokenTo(array_literal); // ']'
        return true;
    }

    if (not Expect(MatchedElemList(array_literal),
                   array_literal,
                   "Expected elem list"
    )) {
        return true;
    }

    // [1, 2, 3,]
    if (CurrentToken().type == TokenType::Op_BracketRight) {
        AddCycledTokenTo(array_literal); // ']'
        return true;
    }

    // if we're here, we have exhausted the elem list and yet haven't found a right bracket.
    // We skip over any potential newlines to avoid leaving the cursor in an awkward state
    // that could result in early parser termination.
    SkipNewlines();

    // That's why we can't Expect() the earlier condition, and we just force an error here
    Expect(false, array_literal, "Expected ']'");

    return true;
}

// grouping = "(" expr ")"
bool Parser::MatchedGrouping(ParseNode& node) {
    if (CurrentToken().type != TokenType::Op_ParenLeft) {
        return false;
    }
    auto& grouping = node.NewBranch();
    grouping.rule  = Rule::Grouping;
    AddCycledTokenTo(grouping);

    if (not Expect(MatchedExpression(grouping),
                   grouping,
                   "Expected expression"
    )) {
        return true;
    }

    if (not Expect(grouping.branches.size() == 1,
                   grouping,
                   "Grouping may not contain more than one expression"
    )) {
        return true;
    }

    if (not Expect(CurrentToken().type == TokenType::Op_ParenRight,
                   grouping,
                   "Expected ')'"
    )) {
        return true;
    }

    AddCycledTokenTo(grouping);
    Expect(grouping.branches.size() == 1,
           grouping,
           "Grouping may not contain more than one expression"
    );

    return true;
}

// literal = number | string | KW_true | KW_false | KW_null
bool IsLiteral(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Lit_Int:
    case Lit_Float:

    case Lit_Char:
    case Lit_String:

    case Lit_true:
    case Lit_false:
    case Lit_none:
        return true;
    default:
        return false;
    }
}

// primary  = grouping | array_literal | literal | ID
bool Parser::MatchedPrimary(ParseNode& node) {
    if (MatchedGrouping(node)) {
        return true;
    }

    if (MatchedArrayLiteral(node)) {
        return true;
    }

    if (IsLiteral(CurrentToken().type)) {
        auto& primary = node.NewBranch(Rule::Literal);
        AddCycledTokenTo(primary);
        return true;
    }

    if (CurrentToken().type == TokenType::Identifier) {
        auto& primary = node.NewBranch(Rule::Identifier);
        AddCycledTokenTo(primary);
        return true;
    }

    return false;
}

// unary = ("-" | "!") unary | primary
bool Parser::MatchedUnary(ParseNode& node) {
    switch (CurrentToken().type) {
        using enum TokenType;

    case Op_Minus:
    case Op_LogicalNot: {
        auto& unary {node.NewBranch(Rule::Unary)};
        AddCycledTokenTo(unary);

        Expect(MatchedUnary(unary),
               unary,
               "Expected resolution into primary expression"
        );
        return true;
    }

    default:
        return MatchedPrimary(node);
    }
}

bool IsFactorOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_FwdSlash:
    case Op_Asterisk:
    case Op_Modulo:
        return true;
    default:
        return false;
    }
}

bool IsTermOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_Minus:
    case Op_Plus:
        return true;
    default:
        return false;
    }
}

bool IsComparisonOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_GreaterThan:
    case Op_GreaterEqual:
    case Op_LessThan:
    case Op_LessEqual:
        return true;
    default:
        return false;
    }
}

bool IsEqualityOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_Equality:
    case Op_NotEqual:
        return true;
    default:
        return false;
    }
}

bool IsLogicalOp(const TokenType token) {
    switch (token) {
        using enum TokenType;

    case Op_LogicalAnd:
    case Op_LogicalOr:
        return true;
    default:
        return false;
    }
}

// expr = logical
bool Parser::MatchedExpression(ParseNode& node) {
    return MatchedLogical(node);
}

// logical = equality ( ('&&' | '||') equality )*
bool Parser::MatchedLogical(ParseNode& node) {
    return MatchedBinaryExpr(node, IsLogicalOp, &Parser::MatchedEquality, Rule::Logical);
}

// equality = comparison (  ('!=' | '==') comparison)*
bool Parser::MatchedEquality(ParseNode& node) {
    return MatchedBinaryExpr(node, IsEqualityOp, &Parser::MatchedComparison, Rule::Equality);
}

// comparison = term ( ('>' | '>=' | '<' | '<=') term)*
bool Parser::MatchedComparison(ParseNode& node) {
    return MatchedBinaryExpr(node, IsComparisonOp, &Parser::MatchedTerm, Rule::Comparison);
}

// term = factor ( ('-' | '+') factor)*
bool Parser::MatchedTerm(ParseNode& node) {
    return MatchedBinaryExpr(node, IsTermOp, &Parser::MatchedFactor, Rule::Term);
}

// factor = unary ( ('/' | '*') unary )*
bool Parser::MatchedFactor(ParseNode& node) {
    return MatchedBinaryExpr(node, IsFactorOp, &Parser::MatchedUnary, Rule::Factor);
}

bool Parser::MatchedBinaryExpr(ParseNode& node,
                               const OpCheckerFnPtr is_valid_operator,
                               const MatcherFnPtr matched_operand,
                               const Rule rule
) {
    if (not(this->*matched_operand)(node)) {
        return false;
    }

    // by this point, we've already created a fresh primary node,
    // so from here on out we don't want to return false and risk destroing AST progress
    // or double parsing
    if (not is_valid_operator(CurrentToken().type)) {
        return true; // if there's no operator following, this is just a primary
    }

    auto& binary_expr = node.NewBranch(rule);
    AddCycledTokenTo(binary_expr);

    // LHS matched, so we need to make it a child of this expr
    const auto lhs_index = node.branches.size() - 2;
    binary_expr.AcquireBranchOf(node, lhs_index);

    // we need to store the index early, as branches may be acquired before we add the rhs
    const auto rhs_index = node.branches.size() - 1;

    if (not Expect((this->*matched_operand)(node),
                   binary_expr,
                   "Expected expression"
    )) {
        return true;
    }

    while (is_valid_operator(CurrentToken().type)) {
        AddCycledTokenTo(binary_expr);

        if (not Expect((this->*matched_operand)(node),
                       binary_expr,
                       "Expected expression"
        )) {
            return true;
        }
    }

    binary_expr.AcquireBranchesOf(node, rhs_index + 1);

    Expect(binary_expr.branches.size() >= 2,
           binary_expr,
           "Expected more operands in binary expression"
    );

    return true;
}
} // namespace sigil
