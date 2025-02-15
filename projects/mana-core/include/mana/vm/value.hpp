#pragma once

#include <mana/literals.hpp>

#include <array>

namespace mana::vm {

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
    Value(const i64 i)
        : type(Type::Int64)
        , as {.int64 = i} {}

    Value(const u64 u)
        : type(Type::Int64)
        , as {.uint64 = u} {}

    Value(const f64 f)
        : type(Type::Float64)
        , as {.float64 = f} {}

    // ReSharper restore CppNonExplicitConvertingConstructor

    // ReSharper disable once CppNotAllPathsReturnValue
    u64 BitCasted() const {
        switch (type) {
        case Type::Int64:
            return std::bit_cast<u64>(as.int64);
        case Type::Uint64:
            return std::bit_cast<u64>(as.uint64);
        case Type::Float64:
            return std::bit_cast<u64>(as.float64);
        }

        return 0;
        // Log->critical("function should not reach this point");
    }

    bool operator==(const Value& other) const {
        if (type != other.type) {
            return false;
        }

        switch (type) {
        case Type::Int64:
            return as.int64 == other.as.int64;
        case Type::Uint64:
            return as.uint64 == other.as.uint64;
        case Type::Float64:
            return as.float64 == other.as.float64;
        }

        return false;
    }

    static f64 FDispatchI(const As val) {
        return static_cast<f64>(val.int64);
    }

    static f64 FDispatchU(const As val) {
        return static_cast<f64>(val.uint64);
    }

    static f64 FDispatchF(const As val) {
        return val.float64;
    }

    static constexpr std::array dispatch_float {
        FDispatchI,
        FDispatchU,
        FDispatchF,
    };

    static i64 IDispatchI(const As val) {
        return val.int64;
    }

    static i64 IDispatchU(const As val) {
        return static_cast<i64>(val.uint64);
    }

    static i64 IDispatchF(const As val) {
        return static_cast<i64>(val.float64);
    }

    static constexpr std::array dispatch_int {
        IDispatchI,
        IDispatchU,
        IDispatchF,
    };

    static u64 UDispatchI(const As val) {
        return static_cast<u64>(val.int64);
    }

    static u64 UDispatchU(const As val) {
        return val.uint64;
    }

    static u64 UDispatchF(const As val) {
        return static_cast<u64>(val.float64);
    }

    static constexpr std::array dispatch_unsigned {
        UDispatchI,
        UDispatchU,
        UDispatchF,
    };

    f64 AsFloat() const {
        return dispatch_float[static_cast<u8>(type)](as);
    }

    i64 AsInt() const {
        return dispatch_int[static_cast<u8>(type)](as);
    }

    u64 AsUint() const {
        return dispatch_unsigned[static_cast<u8>(type)](as);
    }

    void operator+=(const Value& rhs) {
        switch (type) {
        case Type::Float64:
            as.float64 += rhs.AsFloat();
            break;
        case Type::Int64:
            as.int64 += rhs.AsInt();
            break;
        case Type::Uint64:
            as.uint64 += rhs.AsUint();
            break;
        }
    }

    void operator*=(const i64& rhs) {
        switch (type) {
        case Type::Float64:
            as.float64 *= static_cast<f64>(rhs);
            break;
        case Type::Int64:
            as.int64 *= rhs;
            break;
        case Type::Uint64:
            as.uint64 *= static_cast<u64>(rhs);
            break;
        }
    }

private:
    explicit Value(const Type t)
        : type(t) {
        switch (type) {
        case Type::Int64:
            as.int64 = 0;
        case Type::Uint64:
            as.uint64 = 0u;
        case Type::Float64:
            as.float64 = 0.0;
        }
    }

    void WriteBytes(const std::array<u8, sizeof(As)>& bytes) {
        switch (type) {
        case Type::Int64:
            as.int64 = std::bit_cast<i64>(bytes);
        case Type::Uint64:
            as.uint64 = std::bit_cast<u64>(bytes);
        case Type::Float64:
            as.float64 = std::bit_cast<f64>(bytes);
        }
    }
};

}  // namespace mana::vm