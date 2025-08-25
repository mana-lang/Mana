#pragma once

#include "primitive-type.hpp"
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
    friend class Slice;

    union Data {
        i64  as_i64;
        u64  as_u64;
        f64  as_f64;
        bool as_bool;
    };

    Value(i64 i);
    Value(u64 u);
    Value(f64 f);
    Value(bool b);

    MANA_NODISCARD u64 BitCasted() const;

    MANA_NODISCARD PrimitiveType GetType() const;

    MANA_NODISCARD f64  AsFloat() const;
    MANA_NODISCARD i64  AsInt() const;
    MANA_NODISCARD u64  AsUint() const;
    MANA_NODISCARD bool AsBool() const;

    void operator+=(const Value& rhs);
    void operator-=(const Value& rhs);
    void operator*=(const Value& rhs);
    void operator/=(const Value& rhs);

    bool operator>(const Value& rhs) const;
    bool operator>=(const Value& rhs) const;
    bool operator<(const Value& rhs) const;
    bool operator<=(const Value& rhs) const;

    bool operator!() const;

    bool operator==(const Value& other) const;

    void operator*=(const i64& rhs);

    Value()
        : data {nullptr}
        , length(0)
        , type(PrimitiveType::Invalid) {}

    Value(const Value& other);
    Value(Value&& other) noexcept;

    // copy constructs a completely new value on the heap
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    ~Value();

    template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
         || std::is_same_v<T, bool>
    explicit Value(const std::vector<T>& values) : length(values.size()), type(GetManaTypeFrom(T{})) {
        if (length == 0) {
            data = nullptr;
            return;
        }

        data = new Data[length];
        for (u64 i = 0; i < length; ++i) {
            switch (type) {
                case Int64:
                    data[i].as_i64 = values[i];
                    break;
                case Uint64:
                    data[i].as_u64 = values[i];
                case Float64:
                    data[i].as_f64 = values[i];
                case Bool:
                    data[i].as_bool = values[i];
                default:
                    return;
            }
        }
    }

    // Value(const std::vector<i64>& values);
    // Value(const std::vector<u64>& values);
    // Value(const std::vector<f64>& values);
    // Value(const std::vector<bool>& values);
    //
    // void ConstructFromList(const std::vector<Value>& values);

private:
    Data* data;
    u32   length;
    u8    type;

    explicit Value(PrimitiveType t);

    void WriteBytes(const std::array<u8, sizeof(Data)>& bytes);

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

}  // namespace mana::vm