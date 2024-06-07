#pragma once

#include <vector>
#include <memory>

#include <salem/frontend/token.hpp>

namespace salem::ast {

struct node;
using node_ptr = std::unique_ptr<node>;

enum class rule;
struct node {
    rule rule_;
    token_stream tokens_;
    std::vector<node_ptr> branches_;

    node();

    SALEM_NODISCARD auto new_branch(const rule branch_rule) -> const node_ptr& {
        const auto& new_branch = branches_.emplace_back(std::make_unique<node>());
        new_branch->rule_ = branch_rule;
        return new_branch;
    }
};

enum class rule {
    Undefined,
    Mistake,

    Module,
    EofReached,

    Declaration,
    Decl_Import,
    Decl_Module,
    Decl_Access,
    Decl_Function,
    Decl_Global,

    Import_Module,
    Import_Access,
    Import_Alias,

    AccessSpec,

    Param_List,
    Param,

    Type,
    TypeAnnotation,
    TypeAssociation,

    Scope,
    Statement,
    Return,
    Assignment,

    Expression,
    Expr_Operand,
    Expr_Infix,
    Expr_FunctionCall,

    Arguments,

    String,
    Number,

    MemberAccess,

    CompoundAssignment,

    Init_Mut,
    Init_Static,

    UDT,
    UDT_Struct,
    UDT_Pack,
    UDT_Enum,
    UDT_Body,
    UDT_Init,
};

inline node::node()
    : rule_(rule::Undefined)
    , tokens_({})
    //, subnodes_({})
{}

}// namespace salem
