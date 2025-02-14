#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace literals;

enum class Op : u8 {
    Halt,

    Return,

    Push_Float,
    Negate,

    Add,
    Sub,
    Div,
    Mul,
};

}  // namespace mana::vm