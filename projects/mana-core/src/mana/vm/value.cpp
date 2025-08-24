#include <mana/vm/value.hpp>

#include <stdexcept>
#include <utility>

namespace mana::vm {

#ifdef __GNUC__
#    define FUNCSTR std::string(__PRETTY_FUNCTION__)
#else
#    define FUNCSTR std::string(__func__)
#endif

#ifdef MANA_RELEASE
#    define UNREACHABLE() std::unreachable()
#else
#    define UNREACHABLE() \
        throw std::runtime_error(FUNCSTR + std::string(" -- Reached invalid code path"))
#endif

Value::Value(const i64 i)
    : data {.as_i64 = i}
    , length(1)
    , type(static_cast<u8>(Int64)) {}

Value::Value(const u64 u)
    : data {.as_u64 = u}
    , length(1)
    , type(static_cast<u8>(Uint64)) {}

Value::Value(const f64 f)
    : data {.as_f64 = f}
    , length(1)
    , type(static_cast<u8>(Float64)) {}

Value::Value(const bool b)
    : data {.as_bool = b}
    , length(1)
    , type(static_cast<u8>(Bool)) {}

Value::Value(const Type t)
    : length(1)
    , type(static_cast<u8>(t)) {
    switch (type) {
    case Int64:
        data.as_i64 = 0;
        break;
    case Uint64:
        data.as_u64 = 0u;
        break;
    case Float64:
        data.as_f64 = 0.0;
        break;
    case Bool:
        data.as_bool = false;
        break;
    default:
        UNREACHABLE();
    }
}

u64 Value::BitCasted() const {
    switch (type) {
    case Int64:
        return std::bit_cast<u64>(data.as_i64);
    case Uint64:
        return std::bit_cast<u64>(data.as_u64);
    case Float64:
        return std::bit_cast<u64>(data.as_f64);
    case Bool:
        return data.as_bool;  // sobbing and weeping
    default:
        UNREACHABLE();
    }
}

Value::Type Value::GetType() const {
    return static_cast<Type>(type);
}

void Value::WriteBytes(const std::array<u8, sizeof(Data)>& bytes) {
    switch (type) {
    case Int64:
        data.as_i64 = std::bit_cast<i64>(bytes);
        break;
    case Uint64:
        data.as_u64 = std::bit_cast<u64>(bytes);
        break;
    case Float64:
        data.as_f64 = std::bit_cast<f64>(bytes);
        break;
    case Bool:
        data.as_bool = bytes[0];
        break;
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

#define CASE_INT      type_int
#define CASE_UNSIGNED type_unsigned
#define CASE_FLOAT    type_float
#define CASE_BOOL     type_bool

#define COMPUTED_GOTO()                  \
    static constexpr std::array choice { \
        &&CASE_INT,                      \
        &&CASE_UNSIGNED,                 \
        &&CASE_FLOAT,                    \
        &&CASE_BOOL,                     \
    };                                   \
    CHECK_BOUNDS_CGT()                   \
    goto* choice[type]

bool Value::operator==(const Value& other) const {
    COMPUTED_GOTO();

CASE_INT:
    return data.as_i64 == other.AsInt();
CASE_UNSIGNED:
    return data.as_u64 == other.AsUint();
CASE_FLOAT:
    return data.as_f64 == other.AsFloat();
CASE_BOOL:
    return data.as_bool == other.AsBool();
}

void Value::operator*=(const i64& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    data.as_i64 *= rhs;
    return;
CASE_UNSIGNED:
    data.as_u64 *= static_cast<u64>(rhs);
    return;
CASE_FLOAT:
    data.as_f64 *= static_cast<f64>(rhs);
    return;
CASE_BOOL:
    UNREACHABLE();
}

#define CGOTO_OPERATOR_CMP(ret, op)                   \
    ret Value::operator op(const Value& rhs) const {  \
        COMPUTED_GOTO();                              \
    CASE_INT:                                         \
        return data.as_i64 op rhs.AsInt();            \
    CASE_UNSIGNED:                                    \
        return data.as_u64 op rhs.AsUint();           \
    CASE_FLOAT:                                       \
        return data.as_f64 op rhs.AsFloat();          \
    CASE_BOOL:                                        \
        UNREACHABLE();                                \
    }

#define CGOTO_OPERATOR_ARITH_ASSIGN(op)          \
    void Value::operator op(const Value& rhs) {  \
        COMPUTED_GOTO();                         \
    CASE_INT:                                    \
        data.as_i64 op rhs.AsInt();              \
        return;                                  \
    CASE_UNSIGNED:                               \
        data.as_u64 op rhs.AsUint();             \
        return;                                  \
    CASE_FLOAT:                                  \
        data.as_f64 op rhs.AsFloat();            \
        return;                                  \
    CASE_BOOL:                                   \
        UNREACHABLE();                           \
    }

CGOTO_OPERATOR_ARITH_ASSIGN(+=);
CGOTO_OPERATOR_ARITH_ASSIGN(-=);
CGOTO_OPERATOR_ARITH_ASSIGN(*=);
CGOTO_OPERATOR_ARITH_ASSIGN(/=);

CGOTO_OPERATOR_CMP(bool, >);
CGOTO_OPERATOR_CMP(bool, >=);
CGOTO_OPERATOR_CMP(bool, <);
CGOTO_OPERATOR_CMP(bool, <=);

bool Value::operator!() const {
    return not data.as_bool;
}

f64 Value::AsFloat() const {
    return dispatch_float[type](data);
}

i64 Value::AsInt() const {
    return dispatch_int[type](data);
}

u64 Value::AsUint() const {
    return dispatch_unsigned[type](data);
}

bool Value::AsBool() const {
    return dispatch_bool[type](data);
}

// Floats
f64 Value::FDispatchI(const Data val) {
    return static_cast<f64>(val.as_i64);
}

f64 Value::FDispatchU(const Data val) {
    return static_cast<f64>(val.as_u64);
}

f64 Value::FDispatchF(const Data val) {
    return val.as_f64;
}

f64 Value::FDispatchB(const Data val) {
    return val.as_bool;
}

// Integers
i64 Value::IDispatchI(const Data val) {
    return val.as_i64;
}

i64 Value::IDispatchU(const Data val) {
    return static_cast<i64>(val.as_u64);
}

i64 Value::IDispatchF(const Data val) {
    return static_cast<i64>(val.as_f64);
}

i64 Value::IDispatchB(const Data val) {
    return val.as_bool;
}

// Unsigned
u64 Value::UDispatchI(const Data val) {
    return static_cast<u64>(val.as_i64);
}

u64 Value::UDispatchU(const Data val) {
    return val.as_u64;
}

u64 Value::UDispatchF(const Data val) {
    return static_cast<u64>(val.as_f64);
}

u64 Value::UDispatchB(const Data val) {
    return val.as_bool;
}

// Boolean
bool Value::BDispatchI(const Data val) {
    return static_cast<bool>(val.as_i64);
}

bool Value::BDispatchU(const Data val) {
    return static_cast<bool>(val.as_u64);
}

bool Value::BDispatchF(const Data val) {
    return static_cast<bool>(val.as_f64);
}

bool Value::BDispatchB(const Data val) {
    return val.as_bool;
}

}  // namespace mana::vm