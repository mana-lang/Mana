#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/slice.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <fstream>

int main(const int argc, char** argv) {
    // hex::Slice out_slice;
    // out_slice.Write(mana::vm::Op::Constant, out_slice.AddConstant(3.14159265358));
    // out_slice.Write(mana::vm::Op::Constant, out_slice.AddConstant(5415.33926158));
    // out_slice.Write(mana::vm::Op::Constant, out_slice.AddConstant(3.14159265358));
    // out_slice.Write(mana::vm::Op::Add);
    // out_slice.Write(mana::vm::Op::Constant, out_slice.AddConstant(5415.33926158));
    // out_slice.Write(mana::vm::Op::Sub);
    // out_slice.Write(mana::vm::Op::Return);
    //
    // std::ofstream out_file("test_1.mhm", std::ios::binary);
    //
    // const auto output = out_slice.Serialize();
    // out_file.write(reinterpret_cast<const char*>(output.data()), output.size());
    //
    // if (not out_file) {
    //     hex::LogErr("Failed to write to file.");
    //     return -1;
    // }
    //
    // out_file.close();

    std::ifstream in_file("test_1.mhm", std::ios::binary);
    if (not in_file) {
        hex::LogErr("Failed to read file.");
        return -1;
    }
    in_file.seekg(0, std::ios::end);
    const auto file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<mana::literals::u8> raw(file_size);
    in_file.read(reinterpret_cast<char*>(raw.data()), file_size);

    hex::Slice in_slice;
    in_slice.Deserialize(raw);

    PrintBytecode(in_slice);
}

// old main

// using namespace mana::vm;
// hex::Slice out_slice;
//
// const auto a = out_slice.AddConstant(1.2);
// const auto b = out_slice.AddConstant(2.4);
//
// out_slice.Write(Op::Constant, a);
// out_slice.Write(Op::Constant, b);
// out_slice.Write(Op::Add);
// out_slice.Write(Op::Negate);
// out_slice.Write(Op::Constant, out_slice.AddConstant(-12.2));
// out_slice.Write(Op::Mul);
// out_slice.Write(Op::Constant, out_slice.AddConstant(3));
// out_slice.Write(Op::Div);
// out_slice.Write(Op::Constant, b);
// out_slice.Write(Op::Sub);
// out_slice.Write(Op::Return);
//
// hex::Log("Out Slice:");
// PrintBytecode(out_slice);
// hex::Log("");
//
// std::ofstream out_file("test.mhm", std::ios::binary);
// out_file.write(reinterpret_cast<char*>(out_slice.Code().data()), out_slice.Code().size());
//
// hex::VirtualMachine vm;
//
// const auto result = magic_enum::enum_name(vm.Interpret(&out_slice));
// hex::Log("");
// hex::Log("Interpret Result: {}", result);
//
// hex::CommandLineSettings cli(argc, argv);
// cli.Populate();
//
// if (cli.ShouldSayHi()) {
//     hex::Log("Hiii :3c");
// }
//
// const std::string_view s = cli.Opt();
// if (not s.empty()) {
//     hex::Log("I dunno what to do with {}, but it sure looks important!", s);
// }