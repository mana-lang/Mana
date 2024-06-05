#pragma once

#include <vector>
#include <memory>

#include <salem/frontend/token.hpp>

namespace salem::ast {
enum class rule;
struct node {
    rule rule_;
    token_stream tokens_;
    std::vector<std::unique_ptr<node>> subnodes_;

    node();
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

    ModuleAccess,

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
