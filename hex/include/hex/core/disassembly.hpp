#pragma once

namespace hexec {
class ByteCode;
}

namespace hex {
/**
 * @brief Prints Hexe bytecode in a human-readable format.
 */
void PrintBytecode(const hexec::ByteCode& s);
} // namespace hex
