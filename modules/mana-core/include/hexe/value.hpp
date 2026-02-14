#pragma once

#include <mana/literals.hpp>

#include <array>
#include <span>
#include <string_view>
#include <vector>

namespace hexe {
using namespace mana;
using namespace mana::literals;

template <typename T>
concept ValuePrimitiveType = std::is_integral_v<T>
                             || std::is_floating_point_v<T>
                             || std::is_same_v<T, bool>
                             || std::is_same_v<T, std::string_view>;

static constexpr u8 QWORD = 8;
static constexpr u8 DWORD = 4;
static constexpr u8 WORD  = 2;
static constexpr u8 BYTE  = 1;

struct Value {
    friend class ByteCode;

    union Data {
        i64 as_i64;
        u64 as_u64;
        f64 as_f64;

        bool as_bool;
        u8 as_bytes[QWORD];

        enum Type : u8 {
            Int64,
            Uint64,
            Float64,

            Bool,
            String,

            None,

            Invalid = 222,
        };
    };

    using SizeType = u32;

    Value();

    Value(i32 i);
    Value(i64 i);

    Value(u32 u);
    Value(u64 u);

    Value(f64 f);

    Value(bool b);

    Value(std::string_view string);
    Value(u8 vt, SizeType length);
    Value(Data::Type vt, SizeType size);

    Value(const Value& other);
    Value(Value&& other) noexcept;

    Value& operator=(const Data& other);
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    ~Value();

    template <ValuePrimitiveType VT>
    explicit Value(const std::span<VT> values)
        : size_bytes {values.size() * sizeof(Data)},
          type {GetValueTypeFrom(VT {})} {
        const auto length = Length();

        if (length == 0) {
            data = nullptr;
            return;
        }

        data = new Data[length];

        for (u64 i = 0; i < length; ++i) {
            if constexpr (std::is_same_v<VT, bool>) {
                data[i].as_bool = values[i];
            } else if constexpr (std::is_floating_point_v<VT>) {
                data[i].as_f64 = static_cast<f64>(values[i]);
            } else if constexpr (std::is_unsigned_v<VT>) {
                data[i].as_u64 = static_cast<u64>(values[i]);
            } else {
                data[i].as_i64 = static_cast<i64>(values[i]);
            }
        }
    }

    MANA_NODISCARD SizeType Length() const;
    MANA_NODISCARD SizeType ByteLength() const;

    MANA_NODISCARD u64 BitCasted(u32 at) const;

    MANA_NODISCARD Data::Type Type() const;
    MANA_NODISCARD Data Raw() const;

    MANA_NODISCARD f64 AsFloat(i64 index = 0) const;
    MANA_NODISCARD i64 AsInt(i64 index = 0) const;
    MANA_NODISCARD u64 AsUint(i64 index = 0) const;
    MANA_NODISCARD bool AsBool(i64 index = 0) const;
    MANA_NODISCARD std::string_view AsString() const;

    void WriteBytesAt(u32 index, const std::array<u8, QWORD>& bytes) const;

    Value operator+(const Value& rhs) const;
    Value operator-(const Value& rhs) const;
    Value operator*(const Value& rhs) const;
    Value operator/(const Value& rhs) const;
    Value operator%(const Value& rhs) const;

    Value operator-() const;

    void operator+=(const Value& rhs);
    void operator-=(const Value& rhs);
    void operator*=(const Value& rhs);
    void operator/=(const Value& rhs);
    void operator%=(const Value& rhs);

    bool operator>(const Value& rhs) const;
    bool operator>=(const Value& rhs) const;
    bool operator<(const Value& rhs) const;
    bool operator<=(const Value& rhs) const;

    bool operator!() const;

    bool operator==(const Value& other) const;

    void operator*=(const i64& rhs);

    Data& operator[](const u32 index) {
        return data[index];
    }

    const Data& operator[](const u32 index) const {
        return data[index];
    }

private:
    Data::Type GetValueTypeFrom(i64) {
        return Data::Type::Int64;
    }

    Data::Type GetValueTypeFrom(f64) {
        return Data::Type::Float64;
    }

    Data::Type GetValueTypeFrom(u64) {
        return Data::Type::Uint64;
    }

    Data::Type GetValueTypeFrom(bool) {
        return Data::Type::Bool;
    }

    Data* data;
    SizeType size_bytes = sizeof(Data);
    u8 type;

    static constexpr auto SIZE_RAW = sizeof(data) + sizeof(size_bytes) + sizeof(type);

    static i64 IDispatchI(const Data* val);
    static i64 IDispatchU(const Data* val);
    static i64 IDispatchF(const Data* val);
    static i64 IDispatchB(const Data* val);

    static constexpr std::array dispatch_int {
        IDispatchI,
        IDispatchU,
        IDispatchF,
        IDispatchB,
    };

    static u64 UDispatchI(const Data* val);
    static u64 UDispatchU(const Data* val);
    static u64 UDispatchF(const Data* val);
    static u64 UDispatchB(const Data* val);

    static constexpr std::array dispatch_unsigned {
        UDispatchI,
        UDispatchU,
        UDispatchF,
        UDispatchB,
    };

    static f64 FDispatchI(const Data* val);
    static f64 FDispatchU(const Data* val);
    static f64 FDispatchF(const Data* val);
    static f64 FDispatchB(const Data* val);

    static constexpr std::array dispatch_float {
        FDispatchI,
        FDispatchU,
        FDispatchF,
        FDispatchB,
    };

    static bool BDispatchI(const Data* val);
    static bool BDispatchU(const Data* val);
    static bool BDispatchF(const Data* val);
    static bool BDispatchB(const Data* val);

    static constexpr std::array dispatch_bool {
        BDispatchI,
        BDispatchU,
        BDispatchF,
        BDispatchB,
    };
};
} // namespace hexe
