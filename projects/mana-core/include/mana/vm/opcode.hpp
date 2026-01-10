#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace literals;

constexpr u8 BASE_REGISTERS = 128;
constexpr u8 CJMP_OP_BYTES  = 5;
constexpr u8 JMP_OP_BYTES   = 3;


enum class Op : u8 {
    Halt,

    Return,       // Op Reg       -> Place value in Reg
    LoadConstant, // Op Reg Const -> Reg = Constants[Const]
    Move,         // Op Dst Src   -> Dst = Src
    Add,          // Op Dst L R   -> Dst = L + R
    Sub,          // etc.
    Div,
    Mul,

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
};
} // namespace mana::vm
