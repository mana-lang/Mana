#pragma once

#include <hex/ast/token.hpp>

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
    //

    // Expr_Operand,
    // Expr_Infix,
    // Expr_FunctionCall,
    //
    // Arguments,
    //

    //
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

    explicit Node(const Rule r = Rule::Undefined)
        : rule {r}
        , parent {nullptr} {}

    explicit Node(Node* p, const Rule r = Rule::Undefined)
        : rule {r}
        , parent {p} {}

    HEX_NODISCARD Node& NewBranch(const Rule new_rule = Rule::Undefined) {
        // because the module node is the root, it's actually useless to list it as a parent
        return *branches.emplace_back(std::make_shared<Node>(rule == Rule::Module ? nullptr : this, new_rule));
    }

    void PopBranch() {
        branches.pop_back();
    }

    void RemoveBranch(const i64 idx) {
        branches.erase(branches.begin() + idx);
    }

    void RemoveBranchFromTail(const i64 idx) {
        branches.erase(branches.begin() + idx);
    }

    HEX_NODISCARD bool IsRoot() const {
        return parent == nullptr;
    }

private:
    // not intended to be used; this is just to identify a root node that isn't a module node
    Node* parent;
};

}  // namespace hex::ast
