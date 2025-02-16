#pragma once

#include <mana/literals.hpp>

#include <array>

namespace mana::vm {
using namespace literals;

struct Value {
    friend class Slice;

    enum class Type : u8 {
        Int64,
        Uint64,
        Float64,
    };

    Type type;

    union As {
        i64 int64;
        u64 uint64;
        f64 float64;
    } as;

    Value() = delete;

    // ReSharper disable CppNonExplicitConvertingConstructor
    Value(i64 i);

    Value(u64 u);

    Value(f64 f);
    // ReSharper restore CppNonExplicitConvertingConstructor

    // ReSharper disable once CppNotAllPathsReturnValue
    u64 BitCasted() const;

    bool operator==(const Value& other) const;

    static f64                  FDispatchI(As val);
    static f64                  FDispatchU(As val);
    static f64                  FDispatchF(As val);
    static constexpr std::array dispatch_float {
        FDispatchI,
        FDispatchU,
        FDispatchF,
    };

    static i64                  IDispatchI(As val);
    static i64                  IDispatchU(As val);
    static i64                  IDispatchF(As val);
    static constexpr std::array dispatch_int {
        IDispatchI,
        IDispatchU,
        IDispatchF,
    };

    static u64                  UDispatchI(As val);
    static u64                  UDispatchU(As val);
    static u64                  UDispatchF(As val);
    static constexpr std::array dispatch_unsigned {
        UDispatchI,
        UDispatchU,
        UDispatchF,
    };

    f64 AsFloat() const;
    i64 AsInt() const;
    u64 AsUint() const;

    void operator+=(const Value& rhs);
    void operator-=(const Value& rhs);
    void operator*=(const Value& rhs);
    void operator/=(const Value& rhs);

    void operator*=(const i64& rhs);

private:
    explicit Value(Type t);
    void WriteBytes(const std::array<u8, sizeof(As)>& bytes);
};

}  // namespace mana::vm