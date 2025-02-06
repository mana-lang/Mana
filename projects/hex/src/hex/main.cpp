#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <vector>

using namespace mana::literals;
using namespace mana::vm;
using Value = f64;

struct Chunk {
    void Write(Op opcode) {
        code.push_back(static_cast<u8>(opcode));
    }

    void Write(Op opcode, const u8 byte) {
        code.push_back(static_cast<u8>(opcode));
        code.push_back(byte);
    }

    usize AddConstant(const Value value) {
        constants.push_back(value);

        return constants.size() - 1;
    }

    auto Code() const -> const std::vector<u8>& {
        return code;
    }

    Value ConstantAt(const i64 idx) const {
        return constants[idx];
    }

private:
    std::vector<u8>    code;
    std::vector<Value> constants;
};

namespace Instruction {
void Constant(Value constant) {
    hex::Log("{:04} | {} | {}", static_cast<u8>(Op::Constant), magic_enum::enum_name(Op::Constant), constant);
}

void Return() {
    hex::Log("{:04} | {}", static_cast<u8>(Op::Return), magic_enum::enum_name(Op::Return));
}

}  // namespace Instruction

void Execute(const Chunk& c) {
    const auto& code = c.Code();

    for (i64 i = 0; i < code.size(); ++i) {
        switch (static_cast<Op>(code[i])) {
            using enum Op;
        case Constant:
            Instruction::Constant(c.ConstantAt(code[++i]));
            break;
        case Return:
            Instruction::Return();
            break;
        default:
            hex::Log("???");
            break;
        }
    }
}

int main(const int argc, char** argv) {
    Chunk chunk;

    const u8 a = chunk.AddConstant(1.2);
    const u8 b = chunk.AddConstant(2.4);
    chunk.Write(Op::Constant, a);
    chunk.Write(Op::Constant, b);
    chunk.Write(Op::Return);

    Execute(chunk);

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