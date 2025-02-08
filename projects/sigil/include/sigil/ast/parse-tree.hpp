#pragma once

#include <mana/literals.hpp>
#include <sigil/ast/token.hpp>

#include <memory>
#include <vector>

namespace sigil::ast {
namespace ml = mana::literals;
enum class Rule : ml::i64 {
    Undefined,
    Mistake,

    Module,

    Expression,

    Grouping,
    Literal,

    Unary,
    Factor,
    Term,
    Comparison,
    Equality,

    // ReachedEOF,
    //
    // Declaration,
    // Decl_Import,
    // Decl_Access,
    // Decl_Function,
    // Decl_Global,
    //
    // Import_Module,
    // Import_Access,
    // Import_Alias,
    //
    // Access_Spec,
    // Access_Decl,
    //
    // Param,
    // Param_List,
    //
    // Type,
    // Type_Annotation,
    // Type_Association,
    //
    // Scope,
    // Statement,
    // Return,
    // Assignment,
    // Arguments,
    // MemberAccess,
    //
    // CompoundAssignment,
    //
    // Init_Mut,
    // Init_Static,
    //
    // UDT,
    // UDT_Struct,
    // UDT_Pack,
    // UDT_Enum,
    // UDT_Body,
    // UDT_Init,
};

struct ParseNode {
    using NodePtr = std::shared_ptr<ParseNode>;

    Rule                 rule;
    TokenStream          tokens;
    std::vector<NodePtr> branches;

    explicit ParseNode(Rule rule = Rule::Undefined);
    explicit ParseNode(ParseNode* parent, Rule rule = Rule::Undefined);

    SIGIL_NODISCARD ParseNode& NewBranch(Rule new_rule = Rule::Undefined);

    void PopBranch();
    void RemoveBranch(ml::i64 idx);
    void RemoveBranchFromTail(ml::i64 idx);

    SIGIL_NODISCARD bool IsRoot() const;

    void AcquireBranchOf(ParseNode& target, ml::i64 index);
    void AcquireBranchesOf(ParseNode& target, ml::i64 start, ml::i64 end);
    void AcquireBranchesOf(ParseNode& target, ml::i64 start);
    void AcquireTailBranchOf(ParseNode& target);

private:
    // not intended to be used; this is just to identify a root node that isn't a module node
    ParseNode* parent;
};

}  // namespace sigil::ast
