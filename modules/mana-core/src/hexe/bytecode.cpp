#include <hexe/bytecode.hpp>
#include <hexe/logger.hpp>

#include <crc/CRC.h>

#include <stdexcept>


constexpr auto HEADER_DESERIALIZE_ERROR = 727;

namespace hexe {
IndexRange::IndexRange(const i64 init_offset, const i64 range)
    : start(init_offset),
      end(init_offset + range) {
    if (end < start) {
        throw std::runtime_error("IndexRange was out of bounds");
    }
}

i64 ByteCode::Write(const Op opcode) {
    instructions.push_back(static_cast<u8>(opcode));

    CheckInstructionSize();

    latest_opcode = opcode;
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
    latest_opcode = opcode;

    return index;
}

i64 ByteCode::WriteCall(u32 address, u8 register_frame) {
    instructions.push_back(static_cast<u8>(Op::Call));
    instructions.push_back(register_frame);

    for (int i = 0; i < sizeof(u32); ++i) {
        instructions.push_back(address & 0xFF);
        address >>= 8;
    }

    CheckInstructionSize();
    latest_opcode = Op::Call;

    constexpr auto offset = sizeof(address)
                            + sizeof(register_frame);

    return instructions.size() - offset;
}

Op ByteCode::LatestOpcode() const {
    return latest_opcode;
}

void ByteCode::SetEntryPoint(const i64 address) {
    entry_point = address;
}

i64 ByteCode::EntryPointValue() const {
    return entry_point;
}

void ByteCode::SetMainRegisterFrame(const u16 window) {
    main_frame = window;
}

u16 ByteCode::MainRegisterFrame() const {
    return main_frame;
}

u8* ByteCode::EntryPoint() {
    return instructions.data() + entry_point;
}

void ByteCode::Patch(const i64 instruction_index, const u16 new_value, const u8 payload_offset) {
    // add 1 to skip past patched op instruction
    const i64 payload = 1 + instruction_index + payload_offset * 2;

    if (payload + 1 >= instructions.size()) {
        Log->critical("Internal Compiler Error");
        Log->error(
            "Attempted to patch instruction at nonexistent index {} "
            "with payload requiring access to {}. Instruction size: {}",
            instruction_index,
            payload + 1,
            instructions.size()
        );

        return;
    }

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

i64 ByteCode::CurrentAddress() const {
    return static_cast<i64>(instructions.size());
}

const std::vector<Value>& ByteCode::Constants() const {
    return constant_pool;
}

std::vector<u8> ByteCode::Serialize() const {
    if (instructions.empty() && constant_pool.empty()) {
        Log->error("Attempted to serialize empty Bytecode instance.");
        return {};
    }

    const auto code = SerializeCode();

    std::vector<u8> hexecutable = SerializeHeader(code);
    hexecutable.reserve(code.size());

    hexecutable.insert(hexecutable.end(), code.begin(), code.end());

    return hexecutable;
}

std::vector<u8> ByteCode::SerializeCode() const {
    const auto constants_bytes = SerializeConstants();
    const std::vector inst_bytes(instructions.begin(), instructions.end());

    std::vector<u8> code = constants_bytes;
    code.insert(code.end(), inst_bytes.begin(), inst_bytes.end());

    return code;
}

std::vector<u8> ByteCode::SerializeConstants() const {
    std::vector<u8> out;

    CheckConstantPoolSize();

    out.reserve(ConstantPoolBytesCount());
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
    const auto serialize = [&header_bytes, &header]([[maybe_unused]] const auto& value) {
        const i64 size = header_bytes.size();

        // treat header as byte array so we can iterate through it with proper endianness
        for (i64 i = sizeof(value) + size - 1;
             i >= size; --i) {
            header_bytes.push_back(reinterpret_cast<const u8*>(&header)[i]);
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
    serialize(header.main_frame);
    serialize(header.PADDING_COMPAT_);

    return header_bytes;
}

Header ByteCode::CreateHeader(const std::vector<u8>& code) const {
    Header header {
        .magic         = Header::MAGIC,
        .entry_point   = static_cast<u64>(entry_point),
        .code_size     = instructions.size(),
        .constant_size = ConstantPoolBytesCount(),
        .checksum      = Checksum(code.data(), code.size()),
        .version_major = Header::VERSION_MAJOR,
        .version_minor = Header::VERSION_MINOR,
        .version_patch = Header::VERSION_PATCH,
        .main_frame    = main_frame,
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

Header ByteCode::DeserializeHeader(const std::vector<u8>& header_bytes) const {
    i64 offset = 0;

    const auto deserialize_header = [&header_bytes, &offset]<typename T>(T& value) {
        std::array<u8, sizeof(T)> field_bytes {};
        const i64 target = sizeof(value) + offset;
        for (i64 up = offset, down = field_bytes.size() - 1;
             up < target;
             ++up, --down
        ) {
            field_bytes[down] = header_bytes[up];
        }

        value  = std::bit_cast<T>(field_bytes);
        offset += sizeof(value);
    };

    auto header = Header {};

    // if the magic number is off, there's no point deserializing the rest
    deserialize_header(header.magic);
    if (header.magic != Header::MAGIC) {
        Log->error("Header corrupted.");
        header.magic = HEADER_DESERIALIZE_ERROR;
        return header;
    }

    deserialize_header(header.entry_point);
    deserialize_header(header.code_size);
    deserialize_header(header.constant_size);
    deserialize_header(header.checksum);
    deserialize_header(header.version_major);
    deserialize_header(header.version_minor);
    deserialize_header(header.version_patch);
    deserialize_header(header.main_frame);

    // padding can just be copied 1:1
    for (i64 p = 0, h = offset; h < sizeof(Header); ++h, ++p) {
        header.PADDING_COMPAT_[p] = header_bytes[h];
    }

    if (header.version_major != Header::VERSION_MAJOR
        || header.version_minor != Header::VERSION_MINOR
        || header.version_patch != Header::VERSION_PATCH) {
        Log->error("Header version mismatch -- Hexe v{} | Other: Hexe v{}.{}.{}",
                   Header::Version,
                   header.version_major,
                   header.version_minor,
                   header.version_patch
        );
        header.magic = HEADER_DESERIALIZE_ERROR;
    }

    return header;
}

u32 ByteCode::Checksum(const void* ptr, usize size) const {
    return CRC::Calculate(ptr, size, CRC::CRC_32());
}

bool ByteCode::Deserialize(const std::vector<u8>& bytes) {
    if (bytes.empty()) {
        Log->error("Attempted to deserialize empty sequence.");
        return false;
    }

    constant_pool.clear();
    instructions.clear();

    // validate header
    const auto header = DeserializeHeader({bytes.begin(), bytes.begin() + sizeof(Header)});
    if (header.magic == HEADER_DESERIALIZE_ERROR) {
        Log->error("Failed to deserialize Hexe header.");
        return false;
    }

    const auto control_checksum = Checksum(bytes.data() + sizeof(Header),
                                           bytes.size() - sizeof(Header)
    );
    if (control_checksum != header.checksum) {
        Log->error("Checksum mismatch -- {} | Expected: {}", control_checksum, header.checksum);
        Log->critical("Hexe bytecode file was likely corrupted.");
        return false;
    }

    // actual deserialization section
    instructions.reserve(header.code_size);
    constant_pool.reserve(header.constant_size / Value::SIZE);

    const IndexRange pool_range {
        sizeof(Header),
        header.constant_size,
    };

    std::array<u8, sizeof(Value::Data)> value_bytes {};
    std::array<u8, sizeof(Value::LengthType)> length_bytes {};

    // constant pool first
    for (i64 offset = pool_range.start; offset < pool_range.end;) {
        const auto type = static_cast<PrimitiveValueType>(bytes[offset]);

        offset += sizeof(PrimitiveValueType);
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
        constant_pool.push_back(value);
    }

    instructions.insert(instructions.begin(), bytes.begin() + pool_range.end, bytes.end());

    if (header.entry_point >= instructions.size()) {
        Log->error("Entry point index out of bounds.");
        return false;
    }

    entry_point = header.entry_point;
    main_frame  = header.main_frame;

    return true;
}

void ByteCode::CheckInstructionSize() const {
    /// TODO: ideally we handle this in such a way that we don't need to crash
    /// also i hate exceptions
    /// but also, there's zero chance this should ever happen
    if (instructions.size() >= BYTECODE_INSTRUCTION_MAX) {
        throw std::runtime_error("Bytecode instruction limit exceeded");
    }
}

void ByteCode::CheckConstantPoolSize() const {
    if (ConstantCount() >= BYTECODE_CONSTANT_MAX) {
        throw std::runtime_error("Bytecode constant pool exceeded maximum size");
    }
}
} // namespace mana::hexe
