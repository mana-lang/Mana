#pragma once

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>
#include <mana/vm/value.hpp>

#include <cstdlib>
#include <limits>
#include <vector>

namespace mana::vm {
using namespace literals;

struct IndexRange {
    i64 start, end;

    IndexRange(i64 init_offset, i64 range);
    IndexRange() = delete;
};

using ByteCode = std::vector<u8>;

class Slice {
    ByteCode instructions;

    std::vector<Value> values;

public:
    // returns opcode's index
    u64 Write(Op opcode);

    // returns opcode's index
    u64 Write(Op opcode, std::initializer_list<u16> payloads);

    // Modifies a payload for the given opcode
    // This function exists to amend instruction payloads,
    // and thus it assumes the index given is for the opcode whose payload you wish to patch,
    // not the payload itself
    void Patch(u64 instruction_index, u16 new_value, u8 payload_index = 0);

    MANA_NODISCARD u64 BackIndex() const;

    MANA_NODISCARD const ByteCode& Instructions() const;
    MANA_NODISCARD ByteCode& Instructions();

    MANA_NODISCARD const std::vector<Value>& Constants() const;

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - constant pool size in bytes (u64) -> 8
    // - constant pool
    // --- where (size in bytes) elem:
    // ----- (1) type
    // ----- (4) length
    // ----- (8 * length) value(s)
    // - instructions (u16)
    // ---- must keep in mind Push instructions jump to 2-byte indices
    // ---- constant pool has a max size of 65535 (u16-max)
    MANA_NODISCARD ByteCode Serialize();
    MANA_NODISCARD ByteCode SerializeConstants() const;

    MANA_NODISCARD u64 ConstantPoolBytesCount() const;
    MANA_NODISCARD u64 ConstantCount() const;

    // this function assumes correct input
    bool Deserialize(const ByteCode& bytes);

    template <typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
                 || std::is_same_v<T, bool>
    u16 AddConstant(const T value) {
        values.push_back(value);

        return values.size() - 1;
    }

    template <typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
                 || std::is_same_v<T, bool>
    u16 AddConstants(const std::vector<T>& constants) {
        if (values.size() >= std::numeric_limits<u16>::max()) {
            // error
            std::abort();
        }

        const auto first_elem_index = values.size();

        values.push_back(Value {constants});

        return first_elem_index;
    }
};
} // namespace mana::vm
