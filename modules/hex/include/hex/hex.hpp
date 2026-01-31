#pragma once

#include <mana/literals.hpp>
#include <hexe/bytecode.hpp>

namespace hex {
namespace ml = mana::literals;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class Hex {
    ml::u8* ip {nullptr};

    std::vector<hexe::Value> registers;

public:
    Hex();

    InterpretResult Execute(hexe::ByteCode* next_slice);
};
} // namespace hex
