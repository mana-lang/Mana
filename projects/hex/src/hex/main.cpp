#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/slice.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <vector>

namespace hex {
using namespace mana::literals;
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

int main(const int argc, char** argv) {
    using namespace mana::literals;
    using namespace mana::vm;
    hex::Slice slice;

    const u8 a = slice.AddConstant(1.2);
    const u8 b = slice.AddConstant(2.4);

    slice.Write(Op::Constant, a);
    slice.Write(Op::Constant, b);
    slice.Write(Op::Add);
    slice.Write(Op::Negate);
    slice.Write(Op::Constant, slice.AddConstant(-12.2));
    slice.Write(Op::Mul);
    slice.Write(Op::Constant, slice.AddConstant(3));
    slice.Write(Op::Div);
    slice.Write(Op::Constant, b);
    slice.Write(Op::Sub);
    slice.Write(Op::Return);

    PrintBytecode(slice);

    hex::Log("");

    hex::VirtualMachine vm;

    const auto result = magic_enum::enum_name(vm.Interpret(&slice));
    hex::Log("");
    hex::Log("Interpret Result: {}", result);

    hex::CommandLineSettings cli(argc, argv);
    cli.Populate();

    if (cli.ShouldSayHi()) {
        hex::Log("Hiii :3c");
    }

    const std::string_view s = cli.Opt();
    if (not s.empty()) {
        hex::Log("I dunno what to do with {}, but it sure looks important!", s);
    }
}