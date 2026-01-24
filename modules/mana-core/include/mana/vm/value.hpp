#pragma once

#include <mana/vm/primitive-type.hpp>
#include <mana/literals.hpp>

#include <array>
#include <vector>

namespace mana::vm {
using namespace literals;

inline PrimitiveType GetManaTypeFrom(i64) {
    return Int64;
}

inline PrimitiveType GetManaTypeFrom(f64) {
    return Float64;
}

inline PrimitiveType GetManaTypeFrom(u64) {
    return Uint64;
}

inline PrimitiveType GetManaTypeFrom(bool) {
    return Bool;
}

struct Value {
    friend class ByteCode;

    union Data {
        i64 as_i64;
        u64 as_u64;
        f64 as_f64;
        bool as_bool;
    };

    using LengthType = u32;

    Value(i64 i);
    Value(u64 u);
    Value(f64 f);
    Value(bool b);

    // These constructors exist only to redirect to their long/64-bit variants
    // To avoid having to be explicit when initializing Values
    Value(i32 i);
    Value(u32 u);

    MANA_NODISCARD LengthType Length() const;
    MANA_NODISCARD u64 BitCasted(u32 at) const;

    MANA_NODISCARD PrimitiveType GetType() const;

    MANA_NODISCARD f64 AsFloat() const;
    MANA_NODISCARD i64 AsInt() const;
    MANA_NODISCARD u64 AsUint() const;
    MANA_NODISCARD bool AsBool() const;

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

    Value()
        : data {nullptr},
          length(0),
          type(Invalid) {}

    Value(const Value& other);
    Value(Value&& other) noexcept;

    // copy constructs a completely new value on the heap
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    ~Value();

    template <typename T>
        requires std::is_integral_v<T>
                 || std::is_floating_point_v<T>
                 || std::is_same_v<T, bool>
    explicit Value(const std::vector<T>& values)
        : length(values.size()),
          type(GetManaTypeFrom(T {})) {
        if (length == 0) {
            data = nullptr;
            return;
        }

        if (length == 1) {
            data = new Data;
        } else {
            data = new Data[length];
        }

        // init using if constexpr to avoid narrowing warnings
        for (u64 i = 0; i < length; ++i) {
            if constexpr (std::is_same_v<T, bool>) {
                data[i].as_bool = values[i];
            } else if constexpr (std::is_floating_point_v<T>) {
                data[i].as_f64 = static_cast<f64>(values[i]);
            } else if constexpr (std::is_unsigned_v<T>) {
                data[i].as_u64 = static_cast<u64>(values[i]);
            } else {
                data[i].as_i64 = static_cast<i64>(values[i]);
            }
        }
    }

private:
    Data* data;
    LengthType length;
    u8 type;

    Value(PrimitiveType t, LengthType l);

    void WriteValueBytes(const std::array<unsigned char, 8>& bytes, u32 index) const;

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
} // namespace mana::vm
