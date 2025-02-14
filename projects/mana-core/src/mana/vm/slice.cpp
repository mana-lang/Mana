#include <mana/vm/slice.hpp>

namespace mana::vm {

void Slice::Write(Op opcode) {
    instructions.push_back(static_cast<u8>(opcode));
}

void Slice::Write(Op opcode, const u8 byte) {
    instructions.push_back(static_cast<u8>(opcode));
    instructions.push_back(byte);
}

usize Slice::AddConstant(const f64 value) {
    floats.constants.push_back(value);

    return floats.BackIndex();
}

auto Slice::Instructions() const -> const ByteCode& {
    return instructions;
}

auto Slice::Instructions() -> ByteCode& {
    return instructions;
}

auto Slice::FloatConstants() const -> const std::vector<f64>& {
    return floats.constants;
}

auto Slice::Serialize() const -> ByteCode {
    ByteCode out;

    SerializeValueTo(out, floats.constants.size());
    floats.SerializeTo(out);

    out.insert(out.end(), instructions.begin(), instructions.end());

    return out;
}

bool Slice::Deserialize(const ByteCode& bytes) {
    floats.constants.clear();

    const auto       num_floats = DeserializeValue<u64>(bytes);
    const IndexRange float_bytes {sizeof(num_floats), num_floats * sizeof(f64)};

    floats.Deserialize(bytes, float_bytes);

    instructions.insert(instructions.begin(), bytes.begin() + float_bytes.end, bytes.end());

    return true;
}

}  // namespace mana::vm
