#pragma once

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

#include <vector>

namespace mana::vm {
using namespace literals;

class Slice {
    using ByteCode = std::vector<u8>;
    ByteCode bytecode;

    std::vector<f64>  float_constants;
    std::vector<bool> bool_constants;

public:
    void Write(Op opcode);
    void Write(Op opcode, u8 byte);
    u64  AddConstant(f64 value);

    MANA_NODISCARD auto Bytecode() const -> const ByteCode&;
    MANA_NODISCARD auto Bytecode() -> ByteCode&;
    MANA_NODISCARD auto FloatConstants() const -> const std::vector<f64>&;
    MANA_NODISCARD auto BoolConstants() const -> const std::vector<bool>&;

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - constant pool size
    // - constant pool
    // - opcode size
    // - opcode
    MANA_NODISCARD auto Serialize() const -> ByteCode;

    // for now, this function assumes the input is actually correct
    bool Deserialize(const ByteCode& bytes);
};

}  // namespace mana::vm
