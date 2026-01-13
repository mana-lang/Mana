#pragma once

#include <mana/vm/slice.hpp>

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

namespace hex {
namespace ml = mana::literals;

/**
 * @brief Prints the register-based bytecode in a human-readable format.
 * Matches the new 3-address instruction set (Dst, L, R).
 */
void PrintBytecode(const mana::vm::Slice& s);
} // namespace hex
