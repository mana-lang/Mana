#pragma once

#include <hex/ast/token.hpp>

#include <memory>
#include <vector>

namespace hex::ast {
struct Expr {};

struct BinaryExpr final : Expr {
    Expr* left_;
    Expr* right_;
    Token op_;

    BinaryExpr(Expr* left, const Token& op, Expr* right)
        : left_(left)
        , right_(right)
        , op_(op) {}
};

enum class Rule {
    Undefined,

    Module,

    Expression,

    Literal,
    String,
    Number,

    Unary,

    // Mistake,
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
    using NodePtr = std::unique_ptr<Node>;

    Rule                 rule;
    TokenStream          tokens;
    std::vector<NodePtr> branches;

    Node()
        : rule(Rule::Undefined)
        , tokens({}) {}

    HEX_NODISCARD Node& NewBranch() {
        return *branches.emplace_back(std::make_unique<Node>());
    }
};

}  // namespace hex::ast
