#pragma once

#include <mana/literals.hpp>

#include <vector>
#include <initializer_list>
#include <span>

namespace circe {
using namespace mana::literals;

using Register = u16;

class RegisterFrame {
    // elements up to and including the lock_index are locked
    // elements after that are free and reusable
    // elements between tracked.size() and total are in-use
    std::vector<Register> tracked;

    u32 total      = 0;
    i32 lock_index = -1;

public:
    CIRCE_NODISCARD u16 Total() const;

    CIRCE_NODISCARD Register Allocate();

    CIRCE_NODISCARD std::span<const Register> ViewLocked() const;
 // reserved registers are considered locked
    void Reserve(u16 count);

    void Free(Register reg);
    void Free(std::initializer_list<Register> regs);
    void Free(std::span<Register> regs);

    void Lock(Register reg);

    // unlocked registers are considered freed
    void Unlock(Register reg);

    CIRCE_NODISCARD bool IsLocked(Register reg) const;
};
} // namespace circe
