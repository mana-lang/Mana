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
        branches.erase(branches.end() - idx);
    }

    HEX_NODISCARD bool IsRoot() const {
        return parent == nullptr;
    }

    void AcquireBranchOf(Node& target, const i64 index) {
#ifdef HEX_DEBUG
        if (target.branches[index].get() == this) {
            LogErr("Can not acquire branches of self");
            return;
        }
#endif

        branches.emplace_back(target.branches[index]);
        target.RemoveBranch(index);
    }

    void AcquireBranchesOf(Node& target, const i64 start, const i64 end) {
        for (i64 i = start; i <= end; ++i) {
#ifdef HEX_DEBUG
            if (target.branches[i].get() == this) {
                LogErr("Can not acquire branches of self");
                return;
            }
#endif

            branches.emplace_back(target.branches[i]);
        }

        // erase removes up to the penultimate element, but we want inclusive removal
        target.branches.erase(target.branches.begin() + start, target.branches.begin() + end + 1);
    }

    void AcquireBranchesOf(Node& target, const i64 start) {
        for (i64 i = start; i < target.branches.size(); ++i) {
#ifdef HEX_DEBUG
            if (target.branches[i].get() == this) {
                LogErr("Can not acquire branches of self");
                return;
            }
#endif

            branches.emplace_back(target.branches[i]);
        }
        target.branches.erase(target.branches.begin() + start, target.branches.end());
    }

    void AcquireTailBranchOf(Node& target) {
#ifdef HEX_DEBUG
        if (target.branches.back().get() == this) {
            LogErr("Can not acquire branches of self");
            return;
        }
#endif
        branches.emplace_back(target.branches.back());
        target.branches.erase(target.branches.end() - 1);
    }

private:
    // not intended to be used; this is just to identify a root node that isn't a module node
    Node* parent;
};

}  // namespace hex::ast
