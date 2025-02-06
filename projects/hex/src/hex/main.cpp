#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>
#include <hex/slice.hpp>

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

void PrintBytecode(const Slice& c) {
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

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class VirtualMachine {
public:
    InterpretResult interpret(Slice* next_slice) {
        slice = next_slice;
        ip    = &slice->Code()[0];

        return Run();
    }

    InterpretResult Run() {
        while (true) {
            switch (const auto instruction = static_cast<Op>(*ip++)) {
            case Op::Constant: {
                Value constant = slice->ConstantAt(*ip++);
                Log("Constant: {}", constant);
                break;
            }
            case Op::Return:
                Log("Returning.");
                return InterpretResult::OK;
            default:
                return InterpretResult::RuntimeError;
            }
        }
    }

private:
    Slice* slice {nullptr};
    u8*    ip {nullptr};
};

}  // namespace hex

int main(const int argc, char** argv) {
    using namespace mana::literals;
    using namespace mana::vm;
    hex::Slice slice;

    const u8 a = slice.AddConstant(1.2);
    const u8 b = slice.AddConstant(2.4);
    slice.Write(Op::Constant, a);
    slice.Write(Op::Constant, b);
    // slice.Write(Op::Return);

    // PrintBytecode(chunk);

    hex::VirtualMachine vm;
    hex::Log("Interpret Result: {}", magic_enum::enum_name(vm.interpret(&slice)));

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