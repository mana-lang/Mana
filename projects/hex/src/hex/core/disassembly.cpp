#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, f64 constant) {
    Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push), constant);
}

void EmitConstant(i64 offset, const Value constant) {
    const auto print = [offset]<typename T>(decltype(Value::type) type, T val) {
        Log->debug("{:04} | {} | {} | {}", offset, magic_enum::enum_name(Op::Push), magic_enum::enum_name(static_cast<Value::Type>(type)), val);
    };

    switch (constant.type) {
    case Value::Type::Float64:
        print(constant.type, constant.as.float64);
        break;
    case Value::Type::Int64:
        print(constant.type, constant.as.int64);
        break;
    case Value::Type::Uint64:
        print(constant.type, constant.as.uint64);
        break;
    case Value::Type::Boolean:
        print(constant.type, constant.as.boolean);
        break;
    default:
        break;
    }
}

void EmitSimple(i64 offset, const Op op) {
    Log->debug("{:04} | {}", offset, magic_enum::enum_name(op));
}

void PrintBytecode(const Slice& c) {
    const auto& code = c.Instructions();

    for (i64 i = 0; i < code.size(); ++i) {
        switch (const auto op = static_cast<Op>(code[i])) {
            using enum Op;
        case Push:
            EmitConstant(i, c.Constants()[code[i + 1]]);
            ++i;
            break;
        case Negate:
        case Add:
        case Sub:
        case Div:
        case Mul:
        case Halt:
        case Return:
        case Cmp_Greater:
            EmitSimple(i, op);
            break;
        default:
            Log->debug("???");
            break;
        }
    }
}
}  // namespace hex