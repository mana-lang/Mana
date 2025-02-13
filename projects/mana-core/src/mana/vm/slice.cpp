#include <mana/vm/slice.hpp>

#include <cstring>

namespace mana::vm {

void Slice::Write(Op opcode) {
    bytecode.push_back(static_cast<u8>(opcode));
}

void Slice::Write(Op opcode, const u8 byte) {
    bytecode.push_back(static_cast<u8>(opcode));
    bytecode.push_back(byte);
}

usize Slice::AddConstant(const f64 value) {
    float_constants.push_back(value);

    return float_constants.size() - 1;
}

auto Slice::Bytecode() const -> const ByteCode& {
    return bytecode;
}

auto Slice::Bytecode() -> ByteCode& {
    return bytecode;
}

auto Slice::FloatConstants() const -> const std::vector<f64>& {
    return float_constants;
}

auto Slice::Serialize() const -> ByteCode {
    constexpr auto size_elem = sizeof(u64);

    const auto floats_count = float_constants.size();
    const auto floats_bytes = sizeof(f64) * floats_count;

    const auto code_count = bytecode.size();
    const auto code_bytes = sizeof(u8) * code_count;

    // ----------------- we also need store 3 slots for array sizes
    ByteCode ret((size_elem * 3) + floats_bytes + code_bytes);

    std::memcpy(ret.data(), &floats_count, size_elem);
    std::memcpy(ret.data() + size_elem, float_constants.data(), floats_bytes);

    const auto code_offset = size_elem + floats_bytes;
    std::memcpy(ret.data() + code_offset, &code_count, size_elem);
    std::memcpy(ret.data() + code_offset + size_elem, bytecode.data(), code_bytes);

    return ret;
}

bool Slice::Deserialize(const ByteCode& bytes) {
    if (bytes.empty()) {
        // LogErr("Empty vector given to Slice::Deserialize");
        return false;
    }

    float_constants.clear();
    bytecode.clear();

    constexpr auto size_elem = sizeof(u64);

    u64 total_constants = 0;
    std::memcpy(&total_constants, bytes.data(), size_elem);

    const u64 constants_bytes = sizeof(f64) * total_constants;

    float_constants.resize(total_constants);
    std::memcpy(float_constants.data(), bytes.data() + size_elem, constants_bytes);

    u64       total_code  = 0;
    const u64 code_offset = constants_bytes + size_elem;
    std::memcpy(&total_code, bytes.data() + code_offset, size_elem);

    bytecode.resize(total_code);
    std::memcpy(bytecode.data(), bytes.data() + code_offset + size_elem, total_code);

    return true;
}
}  // namespace mana::vm
