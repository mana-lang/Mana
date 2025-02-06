#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/slice.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

int main(const int argc, char** argv) {
    using namespace mana::vm;
    hex::Slice slice;

    const auto a = slice.AddConstant(1.2);
    const auto b = slice.AddConstant(2.4);

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