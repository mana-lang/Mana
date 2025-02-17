#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, f64 constant) {
    Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push), constant);
}

void EmitConstant(i64 offset, const Value constant) {
    const auto print = [offset]<typename T>(const Value::Type type, T val) {
        Log->debug("{:04} | {} | {} | {}", offset, magic_enum::enum_name(Op::Push), magic_enum::enum_name(static_cast<Value::Type>(type)), val);
    };

    switch (constant.GetType()) {
    case Value::Type::Float64:
        print(constant.GetType(), constant.AsFloat());
        break;
    case Value::Type::Int64:
        print(constant.GetType(), constant.AsInt());
        break;
    case Value::Type::Uint64:
        print(constant.GetType(), constant.AsUint());
        break;
    case Value::Type::Bool:
        print(constant.GetType(), constant.AsBool());
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
        case Cmp_Lesser:
        case Cmp_GreaterEq:
        case Cmp_LesserEq:
        case Equals:
        case NotEquals:
            EmitSimple(i, op);
            break;
        default:
            Log->debug("???");
            break;
        }
    }
}
}  // namespace hex