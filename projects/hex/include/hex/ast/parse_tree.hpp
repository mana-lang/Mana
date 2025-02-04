#pragma once

#include <hex/ast/token.hpp>
#include <hex/core/logger.hpp>

#include <memory>
#include <vector>

namespace hex::ast {
enum class Rule : i64 {
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

struct Node {
    using NodePtr = std::shared_ptr<Node>;

    Rule                 rule;
    TokenStream          tokens;
    std::vector<NodePtr> branches;

    explicit Node(Rule r = Rule::Undefined);
    explicit Node(Node* p, Rule r = Rule::Undefined);

    HEX_NODISCARD Node& NewBranch(Rule new_rule = Rule::Undefined);

    void PopBranch();
    void RemoveBranch(i64 idx);
    void RemoveBranchFromTail(i64 idx);

    HEX_NODISCARD bool IsRoot() const;

    void AcquireBranchOf(Node& target, i64 index);
    void AcquireBranchesOf(Node& target, i64 start, i64 end);
    void AcquireBranchesOf(Node& target, i64 start);
    void AcquireTailBranchOf(Node& target);

private:
    // not intended to be used; this is just to identify a root node that isn't a module node
    Node* parent;
};

}  // namespace hex::ast
