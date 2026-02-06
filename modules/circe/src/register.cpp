#include <circe/register.hpp>
#include <circe/core/logger.hpp>

namespace circe {
u16 RegisterFrame::Total() const {
    return total;
}

Register RegisterFrame::Allocate() {
    if (lock_index == tracked.size() - 1) {
        return total++;
    }

    const auto slot = tracked.back();
    tracked.pop_back();

    return slot;
}

std::span<const Register> RegisterFrame::ViewLocked() const {
    if (lock_index < 0) {
        return {};
    }

    if (lock_index >= tracked.size()) {
        Log->error("Internal Compiler Error: Lock index out of bounds");
        return {};
    }

    return std::span(tracked.data(), lock_index + 1);
}

void RegisterFrame::Reserve(u16 count) {
    for (u16 i = 0; i < count; ++i) {
        Lock(Allocate());
    }
}

void RegisterFrame::Free(const Register reg) {
    for (const auto r : tracked) {
        if (r == reg) {
            return;
        }
    }

    if (not IsLocked(reg)) {
        tracked.push_back(reg);
    }
}

void RegisterFrame::Free(const std::initializer_list<Register> regs) {
    for (const auto reg : regs) {
        Free(reg);
    }
}

void RegisterFrame::Free(std::span<Register> regs) {
    for (const auto reg : regs) {
        Free(reg);
    }
}

void RegisterFrame::Lock(const Register reg) {
    i64 idx = -1;
    for (i64 i = 0; i < tracked.size(); ++i) {
        if (tracked[i] == reg) {
            idx = i;
            break;;
        }
    }

    if (idx == -1) {
        tracked.push_back(reg);
        idx = tracked.size() - 1;
    }

    tracked[idx]        = tracked[++lock_index];
    tracked[lock_index] = reg;
}

void RegisterFrame::Unlock(const Register reg) {
    for (i64 i = 0; i < tracked.size(); ++i) {
        if (tracked[i] == reg) {
            tracked[i] = tracked[lock_index];

            tracked[lock_index--] = reg;
            return;
        }
    }
}

bool RegisterFrame::IsLocked(const Register reg) const {
    for (i64 i = 0; i <= lock_index; ++i) {
        if (tracked[i] == reg) {
            return true;
        }
    }
    return false;
}
} // namespace circe
