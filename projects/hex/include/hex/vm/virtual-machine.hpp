#pragma once

#include <hex/vm/stack.hpp>

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
    mvm::Slice* slice {nullptr};
    ml::u8*     ip {nullptr};

    Stack<ml::f64> stack_float {};

public:
    VirtualMachine() = default;

    InterpretResult Interpret(mvm::Slice* next_slice);
};

}  // namespace hex
