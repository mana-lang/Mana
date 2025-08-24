#pragma once

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>
#include <mana/vm/value.hpp>

#include <vector>

namespace mana::vm {
using namespace literals;

struct IndexRange {
    u64 start, end;

    IndexRange(u64 init_offset, u64 range);
    IndexRange() = delete;
};

using ByteCode = std::vector<u8>;

class Slice {
    ByteCode instructions;

    std::vector<Value> values;

public:
    void Write(Op opcode);
    void Write(Op opcode, u8 byte);

    MANA_NODISCARD const ByteCode& Instructions() const;
    MANA_NODISCARD ByteCode&       Instructions();

    MANA_NODISCARD const std::vector<Value>& Constants() const;

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - constant pool size
    // - constant pool
    // --- where (size_bytes) elem:
    // ----- (1) type
    // ----- (8) value
    // - instructions
    MANA_NODISCARD ByteCode Serialize();
    MANA_NODISCARD ByteCode SerializeConstants() const;

    // for now, this function assumes the input is actually correct
    bool Deserialize(const ByteCode& bytes);

    template <typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
                 || std::is_same_v<T, bool>
    u64 AddConstant(const T value) {
        values.push_back(value);

        return values.size() - 1;
    }
};

}  // namespace mana::vm
