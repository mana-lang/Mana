#pragma once

#include <mana/literals.hpp>

#include <emhash/emhash8.hpp>

#include <vector>
#include <initializer_list>
#include <span>

namespace circe {
using namespace mana::literals;

using Register = u16;

class RegisterFrame {
    std::vector<Register> tracked;

    u32 total      = 0;
    i32 lock_index = -1;

public:
    CIRCE_NODISCARD u16 Total() const;

    CIRCE_NODISCARD Register Allocate();

    void Free(Register reg);
    void Free(std::initializer_list<Register> regs);
    void Free(std::span<Register> regs);

    void Lock(Register reg);
    void Unlock(Register reg);

    CIRCE_NODISCARD bool IsLocked(Register reg) const;
};
} // namespace circe
