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

    union As {
        i64  int64;
        u64  uint64;
        f64  float64;
        bool boolean;
    };

    // ReSharper disable CppNonExplicitConvertingConstructor
    Value(i64 i);
    Value(u64 u);
    Value(f64 f);
    Value(bool b);
    // ReSharper restore CppNonExplicitConvertingConstructor

    // ReSharper disable once CppNotAllPathsReturnValue
    u64 BitCasted() const;

    Type GetType() const;

    f64  AsFloat() const;
    i64  AsInt() const;
    u64  AsUint() const;
    bool AsBool() const;

    void operator+=(const Value& rhs);
    void operator-=(const Value& rhs);
    void operator*=(const Value& rhs);
    void operator/=(const Value& rhs);

    bool operator>(const Value& rhs) const;
    bool operator>=(const Value& rhs) const;
    bool operator<(const Value& rhs) const;
    bool operator<=(const Value& rhs) const;

    bool operator==(const Value& other) const;

    void operator*=(const i64& rhs);

private:
    As as;
    u8 type;

    Value() = delete;
    explicit Value(Type t);

    void WriteBytes(const std::array<u8, sizeof(As)>& bytes);

    static i64                  IDispatchI(As val);
    static i64                  IDispatchU(As val);
    static i64                  IDispatchF(As val);
    static i64                  IDispatchB(As val);
    static constexpr std::array dispatch_int {
        IDispatchI,
        IDispatchU,
        IDispatchF,
        IDispatchB,
    };

    static u64                  UDispatchI(As val);
    static u64                  UDispatchU(As val);
    static u64                  UDispatchF(As val);
    static u64                  UDispatchB(As val);
    static constexpr std::array dispatch_unsigned {
        UDispatchI,
        UDispatchU,
        UDispatchF,
        UDispatchB,
    };

    static f64                  FDispatchI(As val);
    static f64                  FDispatchU(As val);
    static f64                  FDispatchF(As val);
    static f64                  FDispatchB(As val);
    static constexpr std::array dispatch_float {
        FDispatchI,
        FDispatchU,
        FDispatchF,
        FDispatchB,
    };

    static bool                 BDispatchI(As val);
    static bool                 BDispatchU(As val);
    static bool                 BDispatchF(As val);
    static bool                 BDispatchB(As val);
    static constexpr std::array dispatch_bool {
        BDispatchI,
        BDispatchU,
        BDispatchF,
        BDispatchB,
    };
};

}  // namespace mana::vm