#include <cmath>

#include <hexe/value.hpp>

#include <stdexcept>
#include <cstring>

#include <hexe/logger.hpp>

#include <magic_enum/magic_enum.hpp>

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

constexpr u8 AsByte(ValueType vt) {
    return static_cast<u8>(vt);
}


Value::Value(const i64 i)
    : data {new Data {.as_i64 = i}},
      type(AsByte(Int64)) {}

Value::Value(const u64 u)
    : data {new Data {.as_u64 = u}},
      type(AsByte(Uint64)) {}

Value::Value(const f64 f)
    : data {new Data {.as_f64 = f}},
      type(AsByte(Float64)) {}

Value::Value(const bool b)
    : data {new Data {.as_bool = b}},
      type(AsByte(Bool)) {}

Value::Value(const i32 i)
    : Value(i64 {i}) {}

Value::Value(const u32 u)
    : Value(u64 {u}) {}

Value::SizeType Value::Length() const {
    //rounded division
    return (size_bytes + sizeof(Data) - 1) / sizeof(Data);
}

Value::SizeType Value::ByteLength() const {
    return size_bytes;
}

Value::Value(const ValueType vt, const SizeType size)
    : size_bytes {size},
      type {AsByte(vt)} {
    const auto length = Length();
    if (length == 0 || type == Invalid) {
        data       = nullptr;
        size_bytes = 0;
        return;
    }

    if (length > 1) {
        data = new Data[length];
        return;
    }

    switch (type) {
    case Int64:
        data = new Data {.as_i64 = 0};
        break;
    case Uint64:
        data = new Data {.as_u64 = 0u};
        break;
    case Float64:
        data = new Data {.as_f64 = 0.0};
        break;
    case Bool:
        data = new Data {.as_bool = false};
        break;
    case String:
        data = new Data {.as_string = ""};
        break;
    case None:
        data = nullptr;
        size_bytes = 0;
        break;
    default:
        UNREACHABLE();
    }
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
        return std::bit_cast<u64>(data[at].as_string);
    default:
        UNREACHABLE();
    }
}

ValueType Value::GetType() const {
    return static_cast<ValueType>(type);
}

void Value::WriteValueBytes(const std::array<u8, sizeof(Data)>& bytes,
                            const u32 index
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
}

Value::Value(const Value& other)
    : data(nullptr),
      size_bytes(other.size_bytes),
      type(other.type) {
    if (other.data == nullptr || size_bytes == 0) {
        return;
    }

    const auto length = Length();
    if (length == 1) {
        data = new Data(*other.data);
        return;
    }

    data = new Data[length];
    std::memcpy(data, other.data, other.size_bytes);
}

Value::Value(Value&& other) noexcept
    : data(nullptr),
      size_bytes(other.size_bytes),
      type(other.type) {
    if (other.data == nullptr || size_bytes == 0) {
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

Value& Value::operator=(const Value& other) {
    if (this == &other) {
        return *this;
    }


    if (data != nullptr) {
        if (Length() == 1) {
            delete data;
        } else {
            delete[] data;
        }
    }

    if (other.data == nullptr || other.size_bytes == 0) {
        size_bytes = 0;
        type       = Invalid;
        data       = nullptr;
        return *this;
    }

    size_bytes = other.size_bytes;
    type       = other.type;

    const auto length = other.Length();
    if (length == 1) {
        data = new Data(*other.data);
        return *this;
    }

    data = new Data[length];
    std::memcpy(data, other.data, other.size_bytes);
    return *this;
}

Value& Value::operator=(Value&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (data != nullptr) {
        if (Length() == 1) {
            delete data;
        } else {
            delete[] data;
        }
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
        // not sure what to do here. Value isn't supposed to log things, but we shouldn't
        // throw in a destructor either. the intuition is to delete it just in case, but
        // that might be even more catastrophic so for now, we'll just consider this a crash scenario
        // maybe should add a ManaLogger or even ValueLogger for situations like this
        std::terminate();
    }
#endif

    const auto length = Length();

    if (length == 1) {
        delete data;
        return;
    }

    if (length > 1) {
        delete[] data;
    }
}

Value::Value(const std::string_view s)
    : data {nullptr},
      size_bytes {static_cast<SizeType>(s.size())},
      type {String} {
    if (size_bytes == 0) {
        return;
    }
    const auto length = Length();

    if (length == 1) {
        data = new Data;
    } else {
        data = new Data[length];
    }

    // copy strings in 8 byte segments
    const auto final_elem = (length - 1);
    for (auto i = 0; i < final_elem; ++i) {
        const auto segment = s.substr(i * sizeof(Data), sizeof(Data));
        std::memcpy(&data[i], segment.data(), sizeof(Data));
    }

    const auto offset  = final_elem * sizeof(Data);
    const auto tail    = size_bytes - offset;
    const auto segment = s.substr(offset, tail);
    std::memcpy(&data[final_elem], segment.data(), tail);
}

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

CGOTO_OPERATOR_BIN(Value, +);
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

bool Value::operator!() const {
    return not data->as_bool;
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

std::string Value::AsString() const {
    if (type != String) {
        Log->critical("Attempted to read value of type {} as string",
                      magic_enum::enum_name(static_cast<ValueType>(type))
        );
        throw std::runtime_error("Value::AsString: Bad Call");
    }

    std::string output;

    const auto length   = Length();
    const auto capacity = length * sizeof(Data);
    output.resize_and_overwrite(capacity,
                                [this, length, capacity](char* ptr, std::size_t len) {
                                    for (auto i = 0; i < length; ++i) {
                                        const auto offset = i * sizeof(Data);
                                        std::memcpy(ptr + offset, &data[i], sizeof(Data));
                                    }

                                    const auto tail = capacity - size_bytes;
                                    return len - tail;
                                }
    );

    return output;
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
