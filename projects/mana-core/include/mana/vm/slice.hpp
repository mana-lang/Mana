#pragma once

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

#include <cstring>
#include <vector>

namespace mana::vm {
using namespace literals;

using Value = f64;

class Slice {
    std::vector<u8>    code;
    std::vector<Value> constants;

public:
    void  Write(Op opcode);
    void  Write(Op opcode, u8 byte);
    usize AddConstant(Value value);

    MANA_NODISCARD auto  Code() const -> const std::vector<u8>&;
    MANA_NODISCARD auto  Code() -> std::vector<u8>&;
    MANA_NODISCARD auto  Constants() const -> const std::vector<Value>&;
    MANA_NODISCARD Value ConstantAt(i64 index) const;

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - constant pool size
    // - constant pool
    // - opcode size
    // - opcode
    MANA_NODISCARD auto Serialize() const -> std::vector<u8>;

    // for now, this function assumes the input is actually correct
    bool Deserialize(const std::vector<u8>& bytes);
};

}  // namespace mana::vm
