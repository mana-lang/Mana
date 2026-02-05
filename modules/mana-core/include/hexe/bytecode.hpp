#pragma once

#include <hexe/opcode.hpp>
#include <hexe/value.hpp>

#include <mana/literals.hpp>

#include <format>
#include <limits>
#include <vector>

#include <spdlog/fmt/compile.h>

namespace hex {
class Hex;
}

namespace hexe {
using namespace mana::literals;

struct IndexRange {
    i64 start, end;

    IndexRange(i64 init_offset, i64 range);
    IndexRange() = delete;
};

static constexpr auto BYTECODE_INSTRUCTION_MAX = std::numeric_limits<i64>::max();
static constexpr auto BYTECODE_CONSTANT_MAX    = std::numeric_limits<u16>::max();

static constexpr auto REGISTER_RETURN = 0;
static constexpr auto REGISTER_TOTAL  = 1024;

using namespace fmt::literals;

// @formatter:off
class Header {
    friend class ByteCode;

    static constexpr u64 MAGIC = 0x45584548414e414d; // do not ever change this

    static constexpr u8 VERSION_MAJOR  = 0;
    static constexpr u8 VERSION_MINOR  = 1;
    static constexpr u16 VERSION_PATCH = 0;


public:
    u64 magic;             // fixed value

    u64 entry_point;       // byte offset to entry function (Main)
    u64 code_size;         // size of instruction section in bytes
    u32 constant_size;     // size of constant pool in bytes

    u32 checksum;          // CRC32 of everything after header

    u8 version_major;      // bytecode format version
    u8 version_minor;
    u16 version_patch;

    u16 main_frame;        // register window for global scope + main

    u8 PADDING_COMPAT_[26]; // extra space reserved for forward compatibility

    static constexpr std::string Version = fmt::format("{}.{}.{}"_cf,
                                                   VERSION_MAJOR,
                                                   VERSION_MINOR,
                                                   VERSION_PATCH
    );

    bool operator==(const Header& other) const;
};
// @formatter:on

struct FunctionEntry {
    i64 address;
    u16 register_count;
};

class ByteCode {
    std::vector<u8> instructions;
    std::vector<Value> constant_pool;

    i64 entry_point;
    u16 main_frame;

    Op latest_opcode;

public:
    // returns opcode's index
    i64 Write(Op opcode);

    // returns opcode's index
    i64 Write(Op opcode, std::initializer_list<u16> payloads);

    // returns opcode's index
    i64 WriteCall(u32 address, u8 register_frame);

    Op LatestOpcode() const;

    // sets the program entry point to be the next instruction's index
    void SetEntryPoint(i64 address);
    MANA_NODISCARD i64 EntryPointValue() const;

    void SetMainRegisterFrame(u16 window);
    MANA_NODISCARD u16 MainRegisterFrame() const;

    // Modifies a payload for the given opcode
    // This function exists to amend instruction payloads,
    // and thus it assumes the index given is for the opcode whose payload you wish to patch,
    // not the payload itself
    void Patch(i64 instruction_index, u16 new_value, u8 payload_offset = 0);

    MANA_NODISCARD i64 BackIndex() const;

    MANA_NODISCARD const std::vector<u8>& Instructions() const;

    MANA_NODISCARD i64 CurrentAddress() const;

    MANA_NODISCARD const std::vector<Value>& Constants() const;

    // serializes Hex bytecode to a vector of unsigned char (bytes) in the Hexe format
    // the sequence is:
    // - Hexe Header (64 bytes)
    // - Constant Pool (size specified by Hexe Header)
    // - Instructions (2 bytes each, total specified by Hexe Header)
    MANA_NODISCARD std::vector<u8> Serialize() const;

    MANA_NODISCARD u32 ConstantPoolBytesCount() const;
    MANA_NODISCARD u32 ConstantCount() const;

    // this function assumes correct input
    bool Deserialize(const std::vector<u8>& bytes);

    template <ValuePrimitive VP>
    u16 AddConstant(const VP value) {
        // only need to store unique constants
        for (i64 i = 0; i < constant_pool.size(); ++i) {
            if (constant_pool[i] == value) {
                return i;
            }
        }

        constant_pool.push_back(value);
        CheckConstantPoolSize();
        return constant_pool.size() - 1;
    }

    template <ValuePrimitive CT>
    u16 AddArray(const std::vector<CT>& array) {
        constant_pool.push_back(Value {array});

        CheckConstantPoolSize();
        return constant_pool.size() - 1;
    }

private:
    MANA_NODISCARD Header DeserializeHeader(const std::vector<u8>& header_bytes) const;
    MANA_NODISCARD u32 Checksum(const void* ptr, usize size) const;

    MANA_NODISCARD std::vector<u8> SerializeCode() const;
    MANA_NODISCARD std::vector<u8> SerializeConstants() const;
    MANA_NODISCARD std::vector<u8> SerializeHeader(const std::vector<u8>& code) const;

    MANA_NODISCARD Header CreateHeader(const std::vector<u8>& code) const;

    void CheckInstructionSize() const;
    void CheckConstantPoolSize() const;

    friend class hex::Hex;
    MANA_NODISCARD u8* EntryPoint();
    MANA_NODISCARD std::vector<u8>& Instructions();
};
} // namespace hexe
