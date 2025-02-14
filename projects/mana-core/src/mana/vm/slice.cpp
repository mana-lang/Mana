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

    // SerializeValueTo(out, ints.constants.size());
    // ints.SerializeTo(out);

    out.insert(out.end(), instructions.begin(), instructions.end());

    return out;
}

bool Slice::Deserialize(const ByteCode& bytes) {
    floats.constants.clear();

    auto serialize_pool = [bytes]<typename T>(ConstantPool<T>& pool, const u64 offset) -> u64 {
        const auto       num_constants = DeserializeValue<u64>(bytes, offset);
        const IndexRange pool_bytes {sizeof(num_constants) + offset, num_constants * sizeof(T)};
        pool.Deserialize(bytes, pool_bytes);

        return pool_bytes.end;
    };

    const auto float_offset = serialize_pool(floats, 0);

    instructions.insert(instructions.begin(), bytes.begin() + float_offset, bytes.end());

    return true;
}

}  // namespace mana::vm
