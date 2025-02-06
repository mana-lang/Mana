#pragma once

#include <hex/vm/slice.hpp>
#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

#include <array>

namespace hex {
using namespace mana::literals;
using namespace mana::vm;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

constexpr i64 Stack_Max = 256;

class VirtualMachine {
public:
    VirtualMachine();

    void ResetStack();

    void Push(Value value);

    Value Pop();

    InterpretResult Interpret(Slice* next_slice);

    InterpretResult Run();

    void BinOp();

private:
    Slice* slice {nullptr};
    u8*    ip {nullptr};

    std::array<Value, Stack_Max> stack {};

    Value* stack_top {nullptr};
};

}  // namespace hex
