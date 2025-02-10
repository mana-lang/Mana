#pragma once

#include "../literals.hpp"

namespace mana::vm {

enum class Op : literals::u8 {
    Halt,

    Return,

    Constant,
    Negate,

    Add,
    Sub,
    Div,
    Mul,
};

}  // namespace mana::vm