#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, f64 constant) {
    Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push_Float), constant);
}

void EmitConstant(i64 offset, i64 constant) {
    // Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push_Int), constant);
}

void EmitConstant(i64 offset, u64 constant) {
    // Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push_UInt), constant);
}

void EmitConstant(i64 offset, bool constant) {
    // Log->debug("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Push_Bool), constant);
}

void EmitSimple(i64 offset, const Op op) {
    Log->debug("{:04} | {}", offset, magic_enum::enum_name(op));
}

void PrintBytecode(const Slice& c) {
    const auto& code = c.Instructions();

    for (i64 i = 0; i < code.size(); ++i) {
        switch (const auto op = static_cast<Op>(code[i])) {
            using enum Op;
        case Push_Float:
            EmitConstant(i, c.FloatConstants()[code[i + 1]]);
            ++i;
            break;
        case Negate:
        case Add:
        case Sub:
        case Div:
        case Mul:
        case Halt:
        case Return:
            EmitSimple(i, op);
            break;
        default:
            Log->debug("???");
            break;
        }
    }
}
}  // namespace hex