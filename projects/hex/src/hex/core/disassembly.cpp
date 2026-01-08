#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, f64 constant) {
    Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push), constant);
}

void EmitConstant(i64 offset, const Value& constant) {
    const auto print = [offset]<typename T>(const mana::PrimitiveType type, T val) {
        Log->debug("{:04} | {} | {} | {}",
                   offset,
                   magic_enum::enum_name(Op::Push),
                   magic_enum::enum_name(type), val);
    };

    switch (constant.GetType()) {
    case mana::PrimitiveType::Float64:
        print(constant.GetType(), constant.AsFloat());
        break;
    case mana::PrimitiveType::Int64:
        print(constant.GetType(), constant.AsInt());
        break;
    case mana::PrimitiveType::Uint64:
        print(constant.GetType(), constant.AsUint());
        break;
    case mana::PrimitiveType::Bool:
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
        i64 increment_offset = 0;
        switch (const auto op = static_cast<Op>(code[i])) {
            using enum Op;
        case Push:
            EmitConstant(i, c.Constants()[code[i + 2]]);
            increment_offset += 2;
            break;
        case JumpWhenFalse:
        case JumpWhenTrue:
        case Jump:
            increment_offset += 2;
        case Pop:
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
        case Not:
            EmitSimple(i, op);
            break;
        default:
            Log->debug("???");
            break;
        }

        i += increment_offset;
    }
}
} // namespace hex
