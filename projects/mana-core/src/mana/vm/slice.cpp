#include <mana/vm/slice.hpp>

#include <stdexcept>

namespace mana::vm {

IndexRange::IndexRange(const u64 init_offset, const u64 range)
    : start(init_offset)
    , end(init_offset + range) {
    if (range < init_offset) {
        throw std::runtime_error("IndexRange was out of bounds");
    }
}

void Slice::Write(Op opcode) {
    instructions.push_back(static_cast<u8>(opcode));
}

void Slice::Write(Op opcode, const u8 byte) {
    instructions.push_back(static_cast<u8>(opcode));
    instructions.push_back(byte);
}

auto Slice::Instructions() const -> const ByteCode& {
    return instructions;
}

auto Slice::Instructions() -> ByteCode& {
    return instructions;
}

auto Slice::Constants() const -> const std::vector<Value>& {
    return values;
}

auto Slice::Serialize() -> ByteCode {
    ByteCode out;

    SerializeConstantsTo(out);

    out.insert(out.end(), instructions.begin(), instructions.end());

    return out;
}

void Slice::SerializeConstantsTo(ByteCode& out) const {
    const auto constant_count = std::bit_cast<u64>(values.size());
    for (i64 i = 0; i < sizeof(constant_count); ++i) {
        out.push_back((constant_count >> i * 8) & 0xFF);
    }

    for (const auto fval : values) {
        out.push_back(static_cast<u8>(fval.type));

        const auto serializable = fval.BitCasted();
        for (i64 i = 0; i < sizeof(serializable); ++i) {
            out.emplace_back((serializable >> i * 8) & 0xFF);
        }
    }
}

bool Slice::Deserialize(const ByteCode& bytes) {
    values.clear();

    // bytes taken up by value-count slot in the constant pool
    std::array<u8, sizeof(u64)> count_bytes;
    for (i64 i = 0; i < count_bytes.size(); ++i) {
        count_bytes[i] = bytes[i];
    }

    const IndexRange pool_range {
        sizeof(count_bytes),
        std::bit_cast<u64>(count_bytes) * (sizeof(Value::As) + sizeof(Value::Type)),
    };

    std::array<u8, sizeof(Value::As)> value_bytes;

    for (i64 n = pool_range.start; n < pool_range.end; n += sizeof(Value::As)) {
        auto value = Value {static_cast<Value::Type>(bytes[n])};
        ++n;

        for (i64 i = 0; i < value_bytes.size(); ++i) {
            value_bytes[i] = bytes[i + n];
        }
        value.WriteBytes(value_bytes);
        values.push_back(value);
    }

    instructions.insert(instructions.begin(), bytes.begin() + pool_range.end, bytes.end());

    return true;
}

}  // namespace mana::vm
