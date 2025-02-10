#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>

#include <magic_enum/magic_enum.hpp>

namespace hex {
using namespace mana::vm;

void EmitConstant(i64 offset, Value constant) {
    Log("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Constant), constant);
}

void EmitSimple(i64 offset, const Op op) {
    Log("{:04} | {}", offset, magic_enum::enum_name(op));
}

void PrintBytecode(const Slice& c) {
    const auto& code = c.Code();

    for (i64 i = 0; i < code.size(); ++i) {
        switch (const auto op = static_cast<Op>(code[i])) {
            using enum Op;
        case Constant:
            EmitConstant(i, c.ConstantAt(code[i + 1]));
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
            Log("???");
            break;
        }
    }
}
}  // namespace hex