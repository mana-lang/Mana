#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace mana::literals;

enum class Op : u8 {
    Return = 22,

    Constant,
    Negate,
};

}  // namespace mana::vm