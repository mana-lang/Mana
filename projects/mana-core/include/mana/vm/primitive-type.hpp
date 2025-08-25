#pragma once

#include <mana/literals.hpp>

namespace mana {
enum PrimitiveType : literals::u8 {
    Int64,
    Uint64,
    Float64,
    Bool,

    Null,

    Invalid = 222,
};
}
