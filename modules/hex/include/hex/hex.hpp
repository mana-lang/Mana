#pragma once

#include <mana/literals.hpp>
#include <hexe/bytecode.hpp>

#include <array>

namespace hex {
namespace ml = mana::literals;

static constexpr auto CALL_STACK_SIZE = 1024;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

struct StackFrame {
    ml::u8* ret_addr;
    ml::u8 reg_frame;
};

class Hex {
    std::array<hexe::Value, hexe::REGISTER_TOTAL> registers = {};
    std::array<StackFrame, CALL_STACK_SIZE> call_stack      = {};

    ml::u8* ip               = nullptr;
    ml::i64 frame_offset     = 0;
    ml::i16 current_function = -1;
    ml::u8 call_register     = 0;

public:
    InterpretResult Execute(hexe::ByteCode* next_slice);
};
} // namespace hex
