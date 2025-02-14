#pragma once
#include <mana/literals.hpp>

#include <array>
#include <vector>

namespace mana::vm {
using namespace literals;

using ByteCode = std::vector<u8>;

struct IndexRange {
    u64 start, end;

    IndexRange(const u64 init_offset, const u64 range)
        : start(init_offset)
        , end(init_offset + range) {
        if (range < init_offset) {
            // Log->error("Invalid IndexRange -- end: {} | start: {}", end, start);
        }
    }

    IndexRange() = delete;
};

template <typename T>
consteval T ToSerializable(T) {
    return T();
}

consteval u32 ToSerializable(const f32 v) {
    return v;
}

consteval u64 ToSerializable(const f64 v) {
    return v;
}

consteval u8 ToSerializable(const bool v) {
    return v;
}

template <typename T>
concept SupportedType = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, bool>;

template <SupportedType T>
void SerializeValueTo(std::vector<u8>& dst, T val) {
    const auto v = std::bit_cast<decltype(ToSerializable(T()))>(val);
    for (i64 i = 0; i < sizeof(T); ++i) {
        dst.push_back((v >> i * 8) & 0xFF);
    }
}

template <SupportedType T>
T DeserializeValue(const std::vector<u8>& src, const i64 offset = 0) {
    std::array<u8, sizeof(T)> ret;

    for (i64 i = 0; i < ret.size(); ++i) {
        ret[i] = src[i + offset];
    }

    return std::bit_cast<T>(ret);
}

template <SupportedType T>
class ConstantPool {
    using SerializableT = decltype(ToSerializable(T()));

public:
    std::vector<T> constants;

    void Deserialize(const ByteCode& bc) {
        return Deserialize(bc, {0, bc.size()});
    }

    void Deserialize(const ByteCode& in_bytes, const IndexRange range) {
        if (range.end > in_bytes.size()) {
            // Log->error("Invalid range given.");
            return;
        }

        std::array<u8, sizeof(T)> val_bytes;

        for (i64 n = range.start; n < range.end; n += sizeof(T)) {
            for (i64 i = 0; i < val_bytes.size(); ++i) {
                val_bytes[i] = in_bytes[i + n];
            }
            constants.push_back(std::bit_cast<T>(val_bytes));
        }
    }

    MANA_NODISCARD auto GetSerialized() const -> ByteCode {
        ByteCode out;
        SerializeTo(out);
        return out;
    }

    void SerializeTo(ByteCode& out) const {
        for (const auto c : constants) {
            const auto v = std::bit_cast<SerializableT>(c);

            for (i64 i = 0; i < sizeof(SerializableT); ++i) {
                out.emplace_back((v >> i * 8) & 0xFF);
            }
        }
    }

    MANA_NODISCARD u64 BackIndex() const {
        return constants.size() - 1;
    }
};

}  // namespace mana::vm
