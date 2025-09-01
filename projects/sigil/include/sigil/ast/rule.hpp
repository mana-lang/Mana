#pragma once

#include <mana/literals.hpp>

namespace sigil::ast {
namespace ml = mana::literals;

enum class Rule : ml::u8 {
    Undefined,
    Mistake,

    Artifact,

    Statement,

    Declaration,
    Assignment,

    Expression,

    Grouping,
    Literal,
    ArrayLiteral,
    ElemList,

    Unary,
    Factor,
    Term,
    Comparison,
    Equality,

    // ReachedEOF,
    //
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
    // Return,
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

}  // namespace sigil::ast