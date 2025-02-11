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
    void Write(Op opcode) {
        code.push_back(static_cast<u8>(opcode));
    }

    void Write(Op opcode, const u8 byte) {
        code.push_back(static_cast<u8>(opcode));
        code.push_back(byte);
    }

    usize AddConstant(const Value value) {
        constants.push_back(value);

        return constants.size() - 1;
    }

    auto Code() const -> const std::vector<u8>& {
        return code;
    }

    auto Code() -> std::vector<u8>& {
        return code;
    }

    auto Constants() const -> const std::vector<Value>& {
        return constants;
    }

    MANA_NODISCARD Value ConstantAt(const i64 index) const {
        return constants[index];
    }

    // serializes a slice to a vector of unsigned char (bytes)
    // for now, the sequence is:
    // - constant pool size
    // - constant pool
    // - opcode size
    // - opcode
    MANA_NODISCARD auto Serialize() const -> std::vector<u8> {
        constexpr auto size_elem = sizeof(u64);

        const auto constants_count = constants.size();
        const auto constants_bytes = sizeof(Value) * constants_count;

        const auto code_count = code.size();
        const auto code_bytes = sizeof(u8) * code_count;

        // ----------------- we also need store 2 slots for array sizes
        std::vector<u8> ret((size_elem * 2) + constants_bytes + code_bytes);

        std::memcpy(ret.data(), &constants_count, size_elem);
        std::memcpy(ret.data() + size_elem, constants.data(), constants_bytes);

        const auto code_offset = size_elem + constants_bytes;
        std::memcpy(ret.data() + code_offset, &code_count, size_elem);
        std::memcpy(ret.data() + code_offset + size_elem, code.data(), code_bytes);

        return ret;
    }

    // for now, this function assumes the input is actually correct
    bool Deserialize(const std::vector<u8>& bytes) {
        if (bytes.empty()) {
            // LogErr("Empty vector given to Slice::Deserialize");
            return false;
        }

        constants.clear();
        code.clear();

        constexpr auto size_elem = sizeof(u64);

        u64 total_constants = 0;
        std::memcpy(&total_constants, bytes.data(), size_elem);

        const u64 constants_bytes = sizeof(Value) * total_constants;

        constants.resize(total_constants);
        std::memcpy(constants.data(), bytes.data() + size_elem, constants_bytes);

        u64       total_code  = 0;
        const u64 code_offset = constants_bytes + size_elem;
        std::memcpy(&total_code, bytes.data() + code_offset, size_elem);

        code.resize(total_code);
        std::memcpy(code.data(), bytes.data() + code_offset + size_elem, total_code);

        return true;
    }
};

}  // namespace mana::vm
