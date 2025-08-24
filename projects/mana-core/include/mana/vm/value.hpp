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

    Value()
        : data {nullptr}
        , length(0)
        , type(invalid_type) {}

    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    ~Value();

private:
    Data* data;
    u32   length;
    u16   rc;
    u8    type;

    explicit Value(Type t);

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

    static constexpr u8 invalid_type = 202;
};

}  // namespace mana::vm