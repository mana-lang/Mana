#include <hex/chunk.hpp>
#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <vector>

namespace hex {
using namespace mana::literals;
using namespace mana::vm;

void EmitConstant(i64 offset, Value constant) {
    Log("{:04} | {} | {}", offset, magic_enum::enum_name(Op::Constant), constant);
}

void EmitSimple(i64 offset) {
    Log("{:04} | {}", offset, magic_enum::enum_name(Op::Return));
}

void PrintBytecode(const Chunk& c) {
    const auto& code = c.Code();

    for (i64 i = 0; i < code.size(); ++i) {
        switch (static_cast<Op>(code[i])) {
            using enum Op;
        case Constant:
            EmitConstant(i, c.ConstantAt(code[i + 1]));
            ++i;
            break;
        case Return:
            EmitSimple(i);
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
    hex::Chunk chunk;

    const u8 a = chunk.AddConstant(1.2);
    const u8 b = chunk.AddConstant(2.4);
    chunk.Write(Op::Constant, a);
    chunk.Write(Op::Constant, b);
    chunk.Write(Op::Return);

    PrintBytecode(chunk);

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