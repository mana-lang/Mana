#include <mana/vm/value.hpp>

#include <stdexcept>
#include <utility>

namespace mana::vm {

#ifdef MANA_RELEASE
#    define UNREACHABLE() std::unreachable()
#else
#    define UNREACHABLE() throw std::runtime_error("Reached invalid code path")
#endif

Value::Value(const i64 i)
    : type(static_cast<u8>(Int64))
    , as {.int64 = i} {}

Value::Value(const u64 u)
    : type(static_cast<u8>(Uint64))
    , as {.uint64 = u} {}

Value::Value(const f64 f)
    : type(static_cast<u8>(Float64))
    , as {.float64 = f} {}

Value::Value(const Type t)
    : type(static_cast<u8>(t)) {
    switch (type) {
    case Int64:
        as.int64 = 0;
        break;
    case Uint64:
        as.uint64 = 0u;
        break;
    case Float64:
        as.float64 = 0.0;
        break;
    default:
        UNREACHABLE();
    }
}

u64 Value::BitCasted() const {
    switch (type) {
    case Int64:
        return std::bit_cast<u64>(as.int64);
    case Uint64:
        return std::bit_cast<u64>(as.uint64);
    case Float64:
        return std::bit_cast<u64>(as.float64);
    default:
        UNREACHABLE();
    }
}

bool Value::operator==(const Value& other) const {
    switch (type) {
    case Int64:
        return as.int64 == other.AsInt();
    case Uint64:
        return as.uint64 == other.AsUint();
    case Float64:
        return as.float64 == other.AsFloat();
    default:
        UNREACHABLE();
    }
}

#ifdef MANA_RELEASE
#    define CHECK_BOUNDS_CGT()
#else
#    define CHECK_BOUNDS_CGT()         \
        if (not(type < choice.size())) \
            throw std::runtime_error("Out of bounds Computed Goto access");
#endif

#define COMPUTED_GOTO()                  \
    static constexpr std::array choice { \
        &&type_int,                      \
        &&type_unsigned,                 \
        &&type_float,                    \
    };                                   \
    CHECK_BOUNDS_CGT()                   \
    goto* choice[type]

#define CASE_INT      type_int
#define CASE_UNSIGNED type_unsigned
#define CASE_FLOAT    type_float

void Value::operator+=(const Value& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    as.int64 += rhs.AsInt();
    return;
CASE_UNSIGNED:
    as.uint64 += rhs.AsUint();
    return;
CASE_FLOAT:
    as.float64 += rhs.AsFloat();
    return;
}

void Value::operator-=(const Value& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    as.int64 -= rhs.AsInt();
    return;
CASE_UNSIGNED:
    as.uint64 -= rhs.AsUint();
    return;
CASE_FLOAT:
    as.float64 -= rhs.AsFloat();
    return;
}

void Value::operator*=(const Value& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    as.int64 *= rhs.AsInt();
    return;
CASE_UNSIGNED:
    as.uint64 *= rhs.AsUint();
    return;
CASE_FLOAT:
    as.float64 *= rhs.AsFloat();
    return;
}

void Value::operator/=(const Value& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    as.int64 /= rhs.AsInt();
    return;
CASE_UNSIGNED:
    as.uint64 /= rhs.AsUint();
    return;
CASE_FLOAT:
    as.float64 /= rhs.AsFloat();
    return;
}

void Value::operator*=(const i64& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    as.int64 *= rhs;
    return;
CASE_UNSIGNED:
    as.uint64 *= static_cast<u64>(rhs);
    return;
CASE_FLOAT:
    as.float64 *= static_cast<f64>(rhs);
    return;
}

void Value::WriteBytes(const std::array<u8, sizeof(As)>& bytes) {
    switch (type) {
    case Int64:
        as.int64 = std::bit_cast<i64>(bytes);
        break;
    case Uint64:
        as.uint64 = std::bit_cast<u64>(bytes);
        break;
    case Float64:
        as.float64 = std::bit_cast<f64>(bytes);
        break;
    default:
        UNREACHABLE();
    }
}

f64 Value::AsFloat() const {
    return dispatch_float[type](as);
}

i64 Value::AsInt() const {
    return dispatch_int[type](as);
}

u64 Value::AsUint() const {
    return dispatch_unsigned[type](as);
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

}  // namespace mana::vm