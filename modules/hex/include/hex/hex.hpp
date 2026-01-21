#pragma once

#include <mana/literals.hpp>
#include <mana/vm/bytecode.hpp>

namespace hex {
namespace ml = mana::literals;
namespace mvm = mana::vm;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class Hex {
    ml::u8* ip {nullptr};

    std::vector<mvm::Value> registers;

public:
    Hex();

    InterpretResult Execute(mvm::ByteCode* next_slice);
};
} // namespace hex
