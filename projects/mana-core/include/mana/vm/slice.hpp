#pragma once

#include <mana/literals.hpp>
#include <mana/vm/constant-pool.hpp>
#include <mana/vm/opcode.hpp>

#include <vector>

namespace mana::vm {
using namespace literals;

class Slice {
    ByteCode instructions;

    ConstantPool<f64> floats;
    ConstantPool<i64> ints;

public:
    void Write(Op opcode);
    void Write(Op opcode, u8 byte);
    u64  AddConstant(f64 value);

    MANA_NODISCARD auto Instructions() const -> const ByteCode&;
    MANA_NODISCARD auto Instructions() -> ByteCode&;
    MANA_NODISCARD auto FloatConstants() const -> const std::vector<f64>&;

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - float pool size
    // - float pool
    // - integer pool size
    // - integer pool
    // - instructions
    MANA_NODISCARD auto Serialize() const -> ByteCode;

    // for now, this function assumes the input is actually correct
    bool Deserialize(const ByteCode& bytes);
};

}  // namespace mana::vm
