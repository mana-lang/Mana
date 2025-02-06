#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace mana::literals;

enum class Op : u8 {
    Return,

    Constant,
    Negate,

    Add,
    Sub,
    Div,
    Mul,
};

}  // namespace mana::vm