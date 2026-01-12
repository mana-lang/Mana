#pragma once

#include <mana/literals.hpp>

namespace mana {
enum PrimitiveType : literals::u8 {
    Int64,
    Uint64,
    Float64,
    Bool,

    None,

    Invalid = 222,
};

template <typename F>
auto DispatchPrimitive(PrimitiveType type, F&& f) {
    using namespace literals;

    switch (type) {
    case Int64:
        return f.template operator()<i64>();
    case Uint64:
        return f.template operator()<u64>();
    case Float64:
        return f.template operator()<f64>();
    case Bool:
        return f.template operator()<bool>();
    default:
        return f.template operator()<void>();
    }
}
} // namespace mana
