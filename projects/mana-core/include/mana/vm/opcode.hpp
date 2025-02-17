#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace literals;

enum class Op : u8 {
    Halt,

    Return,

    Push,
    Negate,

    Add,
    Sub,
    Div,
    Mul,

    Cmp_Greater,
    Cmp_Lesser,
};

}  // namespace mana::vm