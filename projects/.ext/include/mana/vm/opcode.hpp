#pragma once

#include "../literals.hpp"

namespace mana::vm {
using namespace mana::literals;

enum class Op : u8 {
    Return = 0,
    Constant,
};

}  // namespace mana::vm