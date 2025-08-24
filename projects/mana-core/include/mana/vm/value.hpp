#pragma once

#include <mana/literals.hpp>

#include <array>

namespace mana::vm {
using namespace literals;

struct Value {
    friend class Slice;

    enum Type : u8 {
        Int64,
        Uint64,
        Float64,
        Bool,
    };

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

    MANA_NODISCARD Type GetType() const;

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

    // Nulled values shouldn't be valid
    // -727 (bool) is a specific signifier of that
    Value()
        : data {.as_i64 = -727}
        , length(0)
        , type(static_cast<u8>(Bool)) {}

private:
    Data data;
    u32 length;
    u8 type;

    explicit Value(Type t);

    void WriteBytes(const std::array<u8, sizeof(Data)>& bytes);

    static i64 IDispatchI(Data val);
    static i64 IDispatchU(Data val);
    static i64 IDispatchF(Data val);
    static i64 IDispatchB(Data val);

    static constexpr std::array dispatch_int {
        IDispatchI,
        IDispatchU,
        IDispatchF,
        IDispatchB,
    };

    static u64 UDispatchI(Data val);
    static u64 UDispatchU(Data val);
    static u64 UDispatchF(Data val);
    static u64 UDispatchB(Data val);

    static constexpr std::array dispatch_unsigned {
        UDispatchI,
        UDispatchU,
        UDispatchF,
        UDispatchB,
    };

    static f64 FDispatchI(Data val);
    static f64 FDispatchU(Data val);
    static f64 FDispatchF(Data val);
    static f64 FDispatchB(Data val);

    static constexpr std::array dispatch_float {
        FDispatchI,
        FDispatchU,
        FDispatchF,
        FDispatchB,
    };

    static bool BDispatchI(Data val);
    static bool BDispatchU(Data val);
    static bool BDispatchF(Data val);
    static bool BDispatchB(Data val);

    static constexpr std::array dispatch_bool {
        BDispatchI,
        BDispatchU,
        BDispatchF,
        BDispatchB,
    };
};

}  // namespace mana::vm