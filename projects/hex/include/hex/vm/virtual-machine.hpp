#pragma once

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace hex {
namespace ml  = mana::literals;
namespace mvm = mana::vm;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class VirtualMachine {
    ml::u8* ip {nullptr};

    std::vector<mvm::Value> registers;

public:
    VirtualMachine();

    InterpretResult Interpret(mvm::Slice* next_slice);
};

}  // namespace hex
