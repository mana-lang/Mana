#pragma once

#include <hex/vm/slice.hpp>

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

#include <vector>

namespace hex {
using namespace mana::literals;
using namespace mana::vm;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class VirtualMachine {
    Slice* slice {nullptr};
    u8*    ip {nullptr};

    std::vector<Value> stack {};

    Value* stack_top {nullptr};

public:
    VirtualMachine();

    InterpretResult Interpret(Slice* next_slice);

private:
    void ResetStack();

    void Push(Value value);

    Value Pop();

    Value StackTop() const;
};

}  // namespace hex
