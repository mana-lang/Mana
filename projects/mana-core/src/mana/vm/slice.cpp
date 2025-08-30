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

void Slice::Write(const Op opcode, const u16 const_pool_index) {
    instructions.push_back(static_cast<u8>(opcode));

    // constant pool indices are little endian 16-bit unsigned
    instructions.push_back((const_pool_index >> 8) & 0xFF);
    instructions.push_back(const_pool_index & 0xFF);
}

const ByteCode& Slice::Instructions() const {
    return instructions;
}

ByteCode& Slice::Instructions() {
    return instructions;
}

const std::vector<Value>& Slice::Constants() const {
    return values;
}

ByteCode Slice::Serialize() {
    ByteCode out = SerializeConstants();

    out.insert(out.end(), instructions.begin(), instructions.end());

    return out;
}

ByteCode Slice::SerializeConstants() const {
    ByteCode out;

    if (ConstantCount() > std::numeric_limits<u16>::max()) {
        throw std::runtime_error("Constant count too large to serialize");
    }

    // serialize all values, including array indices
    const u64 constant_bytes_count = ConstantPoolBytesCount();
    for (i64 i = 0; i < sizeof(constant_bytes_count); ++i) {
        out.push_back((constant_bytes_count >> i * 8) & 0xFF);
    }

    for (const auto& value : values) {
        out.push_back(static_cast<u8>(value.type));

        for (i64 i = 0; i < sizeof(value.length); ++i) {
            out.push_back((value.length >> i * 8) & 0xFF);
        }

        // need to serialize each value separately
        for (i64 i = 0; i < value.length; ++i) {
            const auto serializable = value.BitCasted(i);

            for (i64 k = 0; k < sizeof(serializable); ++k) {
                out.emplace_back((serializable >> k * 8) & 0xFF);
            }
        }
    }

    return out;
}

u64 Slice::ConstantPoolBytesCount() const {
    u64 out = 0;

    for (const auto& value : values) {
        out += value.length * sizeof(Value::Data);
        out += sizeof(value.type);
        out += sizeof(value.length);
    }
    return out;
}

u64 Slice::ConstantCount() const {
    u64 out = 0;

    for (const auto& value : values) {
        out += value.length;
    }
    return out;
}

bool Slice::Deserialize(const ByteCode& bytes) {
    values.clear();

    // bytes taken up by value-count slot in the constant pool
    std::array<u8, sizeof(u64)> count_bytes;
    for (i64 i = 0; i < count_bytes.size(); ++i) {
        count_bytes[i] = bytes[i];
    }

    const IndexRange pool_range {
        sizeof(count_bytes), // start from end of pool count
        std::bit_cast<u64>(count_bytes),
    };

    std::array<u8, sizeof(Value::Data)>       value_bytes;
    std::array<u8, sizeof(Value::LengthType)> length_bytes;

    for (i64 offset = pool_range.start; offset < pool_range.end;) {
        const auto type = static_cast<PrimitiveType>(bytes[offset]);
        offset += sizeof(PrimitiveType);

        for (i64 i = 0; i < length_bytes.size(); ++i) {
            length_bytes[i] = bytes[i + offset];
        }
        const auto length = std::bit_cast<Value::LengthType>(length_bytes);
        offset += sizeof(Value::LengthType);

        auto value = Value {type, length};
        for (u32 i = 0; i < length; ++i) {
            for (i64 k = 0; k < value_bytes.size(); ++k) {
                value_bytes[k] = bytes[k + offset];
            }
            value.WriteValueBytes(value_bytes, i);
            offset += sizeof(Value::Data);
        }
        values.push_back(value);
    }

    instructions.insert(instructions.begin(), bytes.begin() + pool_range.end, bytes.end());

    return true;
}

}  // namespace mana::vm
