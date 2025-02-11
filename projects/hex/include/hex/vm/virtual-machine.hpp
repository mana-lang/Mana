#pragma once

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

#include <vector>

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

    std::vector<mvm::Value> stack {};

    mvm::Value* stack_top {nullptr};

public:
    VirtualMachine();

    InterpretResult Interpret(mvm::Slice* next_slice);

private:
    void ResetStack();

    void Push(mvm::Value value);

    mvm::Value Pop();

    mvm::Value StackTop() const;
};

}  // namespace hex
