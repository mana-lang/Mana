#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, Op op, f64 constant) {
    Log->debug("{:04} | {} => {}", offset, magic_enum::enum_name(op), constant);
}

void EmitJump(i64 offset, Op op, u16 distance) {
    constexpr u16 num_jmp_instructions = 3;
    Log->debug("{:04} | {} => {:04}",
               offset,
               magic_enum::enum_name(op),
               offset + num_jmp_instructions + distance);
}

void EmitConstant(const i64 offset, const Op op, const Value& constant) {
    const auto print = [offset, op]<typename T>(const mana::PrimitiveType type, T val) {
        if (Op::Push == op) {
            Log->debug("{:04} | {} | {} | {}",
                       offset,
                       magic_enum::enum_name(op),
                       magic_enum::enum_name(type), val);

            return;
        }

        Log->debug("{:04} | {} => {}", offset, magic_enum::enum_name(op), val);
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

u16 ReadPayload(const u8 first_byte, const u8 second_byte) {
    const auto ret = static_cast<u16>(first_byte | second_byte << 8);
    return ret;
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
            EmitConstant(i, op, c.Constants()[ReadPayload(code[i + 1], code[i + 2])]);
            i += 2;
            break;
        case JumpWhenFalse:
        case JumpWhenTrue:
        case Jump:
            EmitJump(i, op, ReadPayload(code[i + 1], code[i + 2]));
            i += 2;
            break;
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
    }
}
} // namespace hex
