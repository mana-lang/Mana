#pragma once

#include <mana/literals.hpp>

namespace hexe {
using namespace mana::literals;

constexpr u8 BASE_REGISTERS = 128;
constexpr u8 CJMP_OP_BYTES  = 5;
constexpr u8 JMP_OP_BYTES   = 3;

constexpr u8 CALL_BYTES = 5;

// @formatter:off
enum class Op : u8 {
    Halt,
    Err,

    Return,        // Op Src       -> Place value in return register
    LoadConstant,  // Op Reg Const -> Reg = Constants[Const]
    Move,          // Op Dst Src   -> Dst = Src

    Add,           // Op Dst L R   -> Dst = L + R
    Sub,           // etc.
    Div,
    Mul,
    Mod,

    Negate,        // Op Dst Src   -> Dst = -Src
    Not,           // Op Dst Src   -> Dst = !Src

    Cmp_Greater,   // Op Dst L R   -> Dst = L > R
    Cmp_GreaterEq, // etc.
    Cmp_Lesser,
    Cmp_LesserEq,

    Equals,
    NotEquals,

    Jump,          // Op Offset       -> Jump by Offset
    JumpWhenTrue,  // Op Reg Offset   -> if Reg { ip += Offset }
    JumpWhenFalse, // etc.

    Call,          // Op RF Addr      -> Register Frame (1 byte)
                   //                 == Destination Address (4 bytes)
                   //                 == Record register frame, then jump to function at address.
                   //                 == Upon returning, retval is copied into designated return register and frame is returned to previous position

    Print,         // Op Str          -> Emit `Str` to stdout
    PrintValue,    // Op Str Val      -> Emit 'Str' to stdout with a value argument

    ListCreate,    // Op Ty  Len Reg  -> Creates new Value of type Ty and reserves Len elements at Reg
    ListRead,      // Op Src Idx Dst  -> Copies Src[Idx] into Dst
    ListWrite,     // Op Dst Idx Src  -> Copies Src into Dst[Idx]
};
// @formatter:on
} // namespace hexe
