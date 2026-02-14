#include <hexe/value.hpp>
#include <hexe/logger.hpp>

#include <magic_enum/magic_enum.hpp>

#include <stdexcept>
#include <cstring>

namespace hexe {
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

#ifdef MANA_RELEASE
#    define CHECK_BOUNDS_CGT()
#else
#    define CHECK_BOUNDS_CGT()         \
        if (not(type < choice.size())) \
            throw std::runtime_error("Attempted operation with invalid Value");
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

#define CGOTO_OPERATOR_BIN(ret, op)                   \
    ret Value::operator op(const Value& rhs) const {  \
        COMPUTED_GOTO();                              \
    CASE_INT:                                         \
        return data->as_i64 op rhs.AsInt();           \
    CASE_UNSIGNED:                                    \
        return data->as_u64 op rhs.AsUint();          \
    CASE_FLOAT:                                       \
        return data->as_f64 op rhs.AsFloat();         \
    CASE_BOOL:                                        \
        UNREACHABLE();                                \
    }

#define CGOTO_OPERATOR_BIN_ASSIGN(op)            \
    void Value::operator op(const Value& rhs) {  \
        COMPUTED_GOTO();                         \
    CASE_INT:                                    \
        data->as_i64 op rhs.AsInt();             \
        return;                                  \
    CASE_UNSIGNED:                               \
        data->as_u64 op rhs.AsUint();            \
        return;                                  \
    CASE_FLOAT:                                  \
        data->as_f64 op rhs.AsFloat();           \
        return;                                  \
    CASE_BOOL:                                   \
        UNREACHABLE();                           \
    }

using enum Value::Data::Type;

Value::Value()
    : data {nullptr},
      size_bytes(0),
      type(Invalid) {}

Value::Value(const i32 i)
    : Value {i64 {i}} {}

Value::Value(const i64 i)
    : data {new Data[1] {Data {.as_i64 = i}}},
      type {Int64} {}

Value::Value(const u32 u)
    : Value {u64 {u}} {}

Value::Value(const u64 u)
    : data {new Data[1] {Data {.as_u64 = u}}},
      type {Uint64} {}

Value::Value(const f64 f)
    : data {new Data[1] {Data {.as_f64 = f}}},
      type {Float64} {}

Value::Value(const bool b)
    : data {new Data[1] {Data {.as_bool = b}}},
      type {Bool} {}

Value::Value(const std::string_view string)
    : data {nullptr},
      size_bytes {static_cast<SizeType>(string.size())},
      type {String} {
    if (size_bytes == 0) {
        return;
    }
    const auto length = Length();

    data = new Data[length] {};

    std::memcpy(data, string.data(), size_bytes);
}

Value::Value(const u8 vt, const SizeType length)
    : Value(static_cast<Data::Type>(vt), length * sizeof(Data)) {}

Value::Value(const Data::Type vt, const SizeType size)
    : size_bytes {size},
      type {vt} {
    const auto length = Length();
    if (length == 0 || type == Invalid) {
        data       = nullptr;
        size_bytes = 0;
        return;
    }

    data = new Data[length] {};
}

Value::Value(u8 vt, const Data& other)
    : data {new Data[1] {other}},
      type {vt} {}


Value::Value(const Value& other)
    : data {nullptr},
      size_bytes {other.size_bytes},
      type {other.type} {
    if (other.data == nullptr || other.size_bytes == 0) {
        return;
    }

    data = new Data[other.Length()] {};
    std::memcpy(data, other.data, other.size_bytes);
}

Value::Value(Value&& other) noexcept
    : data {nullptr},
      size_bytes {other.size_bytes},
      type {other.type} {
    if (other.data == nullptr || other.size_bytes == 0) {
        other.size_bytes = 0;
        other.type       = Invalid;
        other.data       = nullptr;
        return;
    }

    data = other.data;

    other.data       = nullptr;
    other.size_bytes = 0;
    other.type       = Invalid;
}

Value& Value::operator=(const Data& other) {
    if (data != nullptr) {
        delete[] data;
    }
    data = new Data[1] {other};
    return *this;
}

Value& Value::operator=(const Value& other) {
    if (this == &other) {
        return *this;
    }

    if (data != nullptr) {
        delete[] data;
    }

    if (other.data == nullptr || other.size_bytes == 0) {
        size_bytes = 0;
        type       = Invalid;
        data       = nullptr;
        return *this;
    }

    size_bytes = other.size_bytes;
    type       = other.type;
    data       = new Data[other.Length()];

    std::memcpy(data, other.data, other.size_bytes);
    return *this;
}

Value& Value::operator=(Value&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (data != nullptr) {
        delete[] data;
    }

    if (other.data == nullptr || other.size_bytes == 0) {
        size_bytes = 0;
        type       = Invalid;
        data       = nullptr;
        return *this;
    }

    size_bytes = other.size_bytes;
    type       = other.type;
    data       = other.data;

    other.data       = nullptr;
    other.size_bytes = 0;
    other.type       = Invalid;

    return *this;
}

Value::~Value() {
    if (data == nullptr) {
        return;
    }

#ifdef MANA_DEBUG
    if (size_bytes == 0) {
        Log->critical("Empty value was non-null in destructor");
        std::terminate();
    }
#endif

    delete[] data;
}

Value::SizeType Value::Length() const {
    // divide and round up
    return (size_bytes + sizeof(Data) - 1) / sizeof(Data);
}

Value::SizeType Value::ByteLength() const {
    return size_bytes;
}

u64 Value::BitCasted(const u32 at) const {
    if (at >= Length()) {
        Log->critical("Internal Compiler Error: Attempted to bitcast out of bounds");
        return SENTINEL_U64;
    }
    switch (type) {
    case Int64:
        return std::bit_cast<u64>(data[at].as_i64);
    case Uint64:
        return std::bit_cast<u64>(data[at].as_u64);
    case Float64:
        return std::bit_cast<u64>(data[at].as_f64);
    case Bool:
        return data[at].as_bool; // sobbing and weeping
    case String:
        return std::bit_cast<u64>(data[at].as_bytes);
    default:
        UNREACHABLE();
    }
}

Value::Data::Type Value::Type() const {
    return static_cast<Data::Type>(type);
}

Value::Data Value::Raw() const {
    return data[0];
}

void Value::WriteBytesAt(const u32 index,
                         const std::array<u8, sizeof(Data)>& bytes
) const {
    if (index >= Length()) {
        throw std::runtime_error("Value::WriteValueBytes: Out of bounds write");
    }

    switch (type) {
    case Int64:
        data[index].as_i64 = std::bit_cast<i64>(bytes);
        break;
    case Uint64:
        data[index].as_u64 = std::bit_cast<u64>(bytes);
        break;
    case Float64:
        data[index].as_f64 = std::bit_cast<f64>(bytes);
        break;
    case Bool:
        data[index].as_bool = bytes[0];
        break;
    case String:
        std::memcpy(&data[index], bytes.data(), sizeof(Data));
        break;
    default:
        UNREACHABLE();
    }
}

CGOTO_OPERATOR_BIN(Value, +)

bool Value::operator==(const Value& other) const {
    if (type != other.type) {
        return false;
    }

    // we don't have logic for comparing arrays quite yet
    // so [1,2,3] == [1,2,3] currently will just eval to false
    if (Length() > 1 || other.Length() > 1) {
        return false;
    }

    COMPUTED_GOTO();

CASE_INT:
    return data->as_i64 == other.AsInt();
CASE_UNSIGNED:
    return data->as_u64 == other.AsUint();
CASE_FLOAT:
    return data->as_f64 == other.AsFloat();
CASE_BOOL:
    return data->as_bool == other.AsBool();
}

void Value::operator*=(const i64& rhs) {
    COMPUTED_GOTO();

CASE_INT:
    data->as_i64 *= rhs;
    return;
CASE_UNSIGNED:
    data->as_u64 *= static_cast<u64>(rhs);
    return;
CASE_FLOAT:
    data->as_f64 *= static_cast<f64>(rhs);
    return;
CASE_BOOL:
    UNREACHABLE();
};
CGOTO_OPERATOR_BIN(Value, -);
CGOTO_OPERATOR_BIN(Value, *);
CGOTO_OPERATOR_BIN(Value, /)

CGOTO_OPERATOR_BIN_ASSIGN(+=);
CGOTO_OPERATOR_BIN_ASSIGN(-=);
CGOTO_OPERATOR_BIN_ASSIGN(*=);
CGOTO_OPERATOR_BIN_ASSIGN(/=);

CGOTO_OPERATOR_BIN(bool, >);
CGOTO_OPERATOR_BIN(bool, >=);
CGOTO_OPERATOR_BIN(bool, <);
CGOTO_OPERATOR_BIN(bool, <=);

f64 Value::AsFloat(const i64 index) const {
    return dispatch_float[type](data + index);
}

i64 Value::AsInt(const i64 index) const {
    return dispatch_int[type](data + index);
}


u64 Value::AsUint(const i64 index) const {
    return dispatch_unsigned[type](data + index);
}

bool Value::AsBool(const i64 index) const {
    return dispatch_bool[type](data + index);
}

std::string_view Value::AsString() const {
    if (type != String) {
        Log->critical("Attempted to read value of type {} as string",
                      magic_enum::enum_name(static_cast<Data::Type>(type))
        );
        throw std::runtime_error("Value::AsString: Bad Call");
    }

    return {reinterpret_cast<char*>(data), size_bytes};
}

Value Value::operator%(const Value& rhs) const {
    COMPUTED_GOTO();
CASE_INT:
    return data->as_i64 % rhs.AsInt();
CASE_UNSIGNED:
    return data->as_u64 % rhs.AsUint();
CASE_FLOAT:
    return std::fmod(data->as_f64, rhs.AsFloat());
CASE_BOOL:
    UNREACHABLE();
}

Value Value::operator-() const {
    COMPUTED_GOTO();

CASE_INT:
    return Value {-data->as_i64};
CASE_UNSIGNED:
    UNREACHABLE();
CASE_FLOAT:
    return Value {-data->as_f64};
CASE_BOOL:
    UNREACHABLE();
}

void Value::operator%=(const Value& rhs) {
    COMPUTED_GOTO();
CASE_INT:
    data->as_i64 %= rhs.AsInt();
    return;
CASE_UNSIGNED:
    data->as_u64 %= rhs.AsUint();
    return;
CASE_FLOAT:
    data->as_f64 = std::fmod(data->as_f64, rhs.AsFloat());
    return;
CASE_BOOL:
    UNREACHABLE();
}

bool Value::operator!() const {
    return not data->as_bool;
}

// Integers
i64 Value::IDispatchI(const Data* val) {
    return val->as_i64;
}

i64 Value::IDispatchU(const Data* val) {
    return static_cast<i64>(val->as_u64);
}

i64 Value::IDispatchF(const Data* val) {
    return static_cast<i64>(val->as_f64);
}

i64 Value::IDispatchB(const Data* val) {
    return val->as_bool;
}

// Unsigned
u64 Value::UDispatchI(const Data* val) {
    return static_cast<u64>(val->as_i64);
}

u64 Value::UDispatchU(const Data* val) {
    return val->as_u64;
}

u64 Value::UDispatchF(const Data* val) {
    return static_cast<u64>(val->as_f64);
}

u64 Value::UDispatchB(const Data* val) {
    return val->as_bool;
}

// Floats
f64 Value::FDispatchI(const Data* val) {
    return static_cast<f64>(val->as_i64);
}

f64 Value::FDispatchU(const Data* val) {
    return static_cast<f64>(val->as_u64);
}

f64 Value::FDispatchF(const Data* val) {
    return val->as_f64;
}

f64 Value::FDispatchB(const Data* val) {
    return val->as_bool;
}

// Boolean
bool Value::BDispatchI(const Data* val) {
    return static_cast<bool>(val->as_i64);
}

bool Value::BDispatchU(const Data* val) {
    return static_cast<bool>(val->as_u64);
}

bool Value::BDispatchF(const Data* val) {
    return static_cast<bool>(val->as_f64);
}

bool Value::BDispatchB(const Data* val) {
    return val->as_bool;
}
} // namespace hexe
