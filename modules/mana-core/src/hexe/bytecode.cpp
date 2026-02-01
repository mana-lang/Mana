#include <hexe/bytecode.hpp>

#include <crc/CRC.h>

#include <stdexcept>

namespace hexe {
IndexRange::IndexRange(const i64 init_offset, const i64 range)
    : start(init_offset),
      end(init_offset + range) {
    if (range < init_offset) {
        throw std::runtime_error("IndexRange was out of bounds");
    }
}

ByteCode::ByteCode() {}

i64 ByteCode::Write(Op opcode) {
    instructions.push_back(static_cast<u8>(opcode));

    CheckInstructionSize();
    return instructions.size() - 1;
}

i64 ByteCode::Write(const Op opcode, const std::initializer_list<u16> payloads) {
    const auto index = instructions.size();
    instructions.push_back(static_cast<u8>(opcode));

    // payloads are 16-bit unsigned little endian
    for (const auto payload : payloads) {
        instructions.push_back(payload & 0xFF);
        instructions.push_back((payload >> 8) & 0xFF);
    }

    CheckInstructionSize();
    return index;
}

// does not perform bounds checking
void ByteCode::Patch(const i64 instruction_index, const u16 new_value, const u8 payload_offset) {
    // add 1 to skip past instruction
    const i64 payload = 1 + instruction_index + payload_offset * 2;

    instructions[payload]     = new_value & 0xFF;
    instructions[payload + 1] = (new_value >> 8) & 0xFF;
}

i64 ByteCode::BackIndex() const {
    return static_cast<i64>(instructions.size()) - 1;
}

const std::vector<u8>& ByteCode::Instructions() const {
    return instructions;
}

std::vector<u8>& ByteCode::Instructions() {
    return instructions;
}

i64 ByteCode::InstructionCount() const {
    return static_cast<i64>(instructions.size());
}

const std::vector<Value>& ByteCode::Constants() const {
    return constant_pool;
}

std::vector<u8> ByteCode::Serialize() const {
    if (instructions.empty() && constant_pool.empty()) {
        throw std::runtime_error("Attempted to serialize with no bytecode available");
    }

    const auto code = SerializeCode();

    std::vector<u8> hexecutable = SerializeHeader(code);
    hexecutable.reserve(code.size());

    hexecutable.insert(hexecutable.end(), code.begin(), code.end());

    return hexecutable;
}

std::vector<u8> ByteCode::SerializeCode() const {
    const auto const_bytes = SerializeConstants();
    const std::vector inst_bytes(instructions.begin(), instructions.end());

    std::vector<u8> code = const_bytes;
    code.insert(code.end(), inst_bytes.begin(), inst_bytes.end());

    return code;
}

std::vector<u8> ByteCode::SerializeConstants() const {
    std::vector<u8> out;

    CheckConstantPoolSize();

    // serialize all values, including array indices
    const u64 constant_bytes_count = ConstantPoolBytesCount();
    out.reserve(constant_bytes_count);
    for (i64 i = 0; i < sizeof(constant_bytes_count); ++i) {
        out.push_back((constant_bytes_count >> i * 8) & 0xFF);
    }

    for (const auto& value : constant_pool) {
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

std::vector<u8> ByteCode::SerializeHeader(const std::vector<u8>& code) const {
    const Header header = CreateHeader(code);

    std::vector<u8> header_bytes;
    header_bytes.reserve(sizeof(Header));

    // little endian, so we have to do it like this
    const auto serialize = [&header_bytes, &header]([[maybe_unused]] const auto& value) {
        const i64 size = header_bytes.size();
        for (i64 i = sizeof(value) + size - 1;
             i >= size; --i) {
            // treat header as array, offset into it byte by byte
            header_bytes.push_back((reinterpret_cast<const u8*>(&header)[i]) & 0xFF);
        }
    };

    serialize(header.magic);
    serialize(header.entry_point);
    serialize(header.code_size);
    serialize(header.constant_size);
    serialize(header.checksum);
    serialize(header.version_major);
    serialize(header.version_minor);
    serialize(header.version_patch);
    serialize(header.PADDING_COMPAT_);

    return header_bytes;
}

Header ByteCode::CreateHeader(const std::vector<u8>& code) const {
    Header header {
        .magic         = Header::MAGIC,
        .entry_point   = 0xFFFFFFFFFFFFFFFF,
        .code_size     = instructions.size(),
        .constant_size = ConstantPoolBytesCount(),
        .checksum      = CRC::Calculate(code.data(), code.size(), CRC::CRC_32()),
        .version_major = Header::VERSION_MAJOR,
        .version_minor = Header::VERSION_MINOR,
        .version_patch = Header::VERSION_PATCH,
    };

    // padding should be all 1's, safer than uninitialized
    std::memset(header.PADDING_COMPAT_, 0xFF, sizeof(header.PADDING_COMPAT_));

    return header;
}

u32 ByteCode::ConstantPoolBytesCount() const {
    u32 out = 0;

    for (const auto& value : constant_pool) {
        out += value.length * sizeof(Value::Data); // num elements
        out += sizeof(value.type);
        out += sizeof(value.length); // array length still has to be included
    }
    return out;
}

// TODO: I think this is not right lol
// unfortunately, we need to figure out arrays in Hex before we can address this
u32 ByteCode::ConstantCount() const {
    u32 out = 0;
    for (const auto& value : constant_pool) {
        out += value.length;
    }
    return out;
}

bool ByteCode::Deserialize(const std::vector<u8>& bytes) {
    if (bytes.empty()) {
        return false;
    }

    constant_pool.clear();

    // bytes taken up by value-count slot in the constant pool
    std::array<u8, sizeof(u64)> count_bytes {};
    for (i64 i = 0; i < count_bytes.size(); ++i) {
        count_bytes[i] = bytes[i];
    }

    const IndexRange pool_range {
        sizeof(count_bytes),
        // start from end of pool count
        std::bit_cast<i64>(count_bytes),
    };

    std::array<u8, sizeof(Value::Data)> value_bytes {};
    std::array<u8, sizeof(Value::LengthType)> length_bytes {};

    for (i64 offset = pool_range.start; offset < pool_range.end;) {
        const auto type = static_cast<PrimitiveType>(bytes[offset]);

        offset += sizeof(PrimitiveType);
        for (i64 i = 0; i < length_bytes.size(); ++i) {
            length_bytes[i] = bytes[i + offset];
        }
        const auto length = std::bit_cast<Value::LengthType>(length_bytes);
        offset            += sizeof(Value::LengthType);

        auto value = Value {type, length};
        for (u32 i = 0; i < length; ++i) {
            for (i64 k = 0; k < value_bytes.size(); ++k) {
                value_bytes[k] = bytes[k + offset];
            }
            value.WriteValueBytes(value_bytes, i);
            offset += sizeof(Value::Data);
        }
        constant_pool.push_back(value);
    }

    instructions.insert(instructions.begin(), bytes.begin() + pool_range.end, bytes.end());

    return true;
}

void ByteCode::CheckInstructionSize() const {
    if (instructions.size() >= BYTECODE_INSTRUCTION_MAX) {
        /// TODO: ideally we handle this in such a way that we don't need to crash
        /// also i hate exceptions
        throw std::runtime_error("Bytecode instruction limit exceeded");
    }
}

void ByteCode::CheckConstantPoolSize() const {
    if (ConstantCount() >= BYTECODE_CONSTANT_MAX) {
        throw std::runtime_error("Bytecode constant pool exceeded maximum size");
    }
}
} // namespace mana::hexe
