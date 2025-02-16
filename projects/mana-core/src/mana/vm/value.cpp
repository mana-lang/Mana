#include <mana/logger.hpp>
#include <mana/vm/value.hpp>

namespace mana::vm {

Value::Value(const i64 i)
    : type(Type::Int64)
    , as {.int64 = i} {}

Value::Value(const u64 u)
    : type(Type::Int64)
    , as {.uint64 = u} {}

Value::Value(const f64 f)
    : type(Type::Float64)
    , as {.float64 = f} {}

u64 Value::BitCasted() const {
    switch (type) {
    case Type::Int64:
        return std::bit_cast<u64>(as.int64);
    case Type::Uint64:
        return std::bit_cast<u64>(as.uint64);
    case Type::Float64:
        return std::bit_cast<u64>(as.float64);
    }

    throw std::runtime_error("Value type enum held invalid value");
    // Log->critical("function should not reach this point");
}

bool Value::operator==(const Value& other) const {
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

f64 Value::FDispatchI(const As val) {
    return static_cast<f64>(val.int64);
}

f64 Value::FDispatchU(const As val) {
    return static_cast<f64>(val.uint64);
}

f64 Value::FDispatchF(const As val) {
    return val.float64;
}

i64 Value::IDispatchI(const As val) {
    return val.int64;
}

i64 Value::IDispatchU(const As val) {
    return static_cast<i64>(val.uint64);
}

i64 Value::IDispatchF(const As val) {
    return static_cast<i64>(val.float64);
}

u64 Value::UDispatchI(const As val) {
    return static_cast<u64>(val.int64);
}

u64 Value::UDispatchU(const As val) {
    return val.uint64;
}

u64 Value::UDispatchF(const As val) {
    return static_cast<u64>(val.float64);
}

f64 Value::AsFloat() const {
    return dispatch_float[static_cast<u8>(type)](as);
}

i64 Value::AsInt() const {
    return dispatch_int[static_cast<u8>(type)](as);
}

u64 Value::AsUint() const {
    return dispatch_unsigned[static_cast<u8>(type)](as);
}

void Value::operator+=(const Value& rhs) {
    static constexpr std::array choice {
        &&type_int,
        &&type_unsigned,
        &&type_float,
    };

    goto* choice[static_cast<u8>(type)];

type_int:
    as.int64 += rhs.AsInt();
    return;
type_unsigned:
    as.uint64 += rhs.AsUint();
    return;
type_float:
    as.float64 += rhs.AsFloat();
    return;
}

void Value::operator-=(const Value& rhs) {
    static constexpr std::array choice {
        &&type_int,
        &&type_unsigned,
        &&type_float,
    };

    goto* choice[static_cast<u8>(type)];

type_int:
    as.int64 -= rhs.AsInt();
    return;
type_unsigned:
    as.uint64 -= rhs.AsUint();
    return;
type_float:
    as.float64 -= rhs.AsFloat();
    return;
}

void Value::operator*=(const Value& rhs) {
    static constexpr std::array choice {
        &&type_int,
        &&type_unsigned,
        &&type_float,
    };

    goto* choice[static_cast<u8>(type)];

type_int:
    as.int64 *= rhs.AsInt();
    return;
type_unsigned:
    as.uint64 *= rhs.AsUint();
    return;
type_float:
    as.float64 *= rhs.AsFloat();
    return;
}

void Value::operator/=(const Value& rhs) {
    static constexpr std::array choice {
        &&type_int,
        &&type_unsigned,
        &&type_float,
    };

    goto* choice[static_cast<u8>(type)];

type_int:
    as.int64 /= rhs.AsInt();
    return;
type_unsigned:
    as.uint64 /= rhs.AsUint();
    return;
type_float:
    as.float64 /= rhs.AsFloat();
    return;
}

void Value::operator*=(const i64& rhs) {
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

Value::Value(const Type t)
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

void Value::WriteBytes(const std::array<u8, sizeof(As)>& bytes) {
    switch (type) {
    case Type::Int64:
        as.int64 = std::bit_cast<i64>(bytes);
    case Type::Uint64:
        as.uint64 = std::bit_cast<u64>(bytes);
    case Type::Float64:
        as.float64 = std::bit_cast<f64>(bytes);
    }
}
}  // namespace mana::vm