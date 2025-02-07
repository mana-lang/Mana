#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/slice.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/opcode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <fstream>

void WriteTestFile() {
    using namespace mana::vm;
    hex::Slice out_slice;

    const auto a = out_slice.AddConstant(1.2);
    const auto b = out_slice.AddConstant(2.4);

    out_slice.Write(Op::Constant, a);
    out_slice.Write(Op::Constant, b);
    out_slice.Write(Op::Add);
    out_slice.Write(Op::Negate);
    out_slice.Write(Op::Constant, out_slice.AddConstant(-12.2));
    out_slice.Write(Op::Mul);
    out_slice.Write(Op::Constant, out_slice.AddConstant(3));
    out_slice.Write(Op::Div);
    out_slice.Write(Op::Constant, b);
    out_slice.Write(Op::Sub);
    out_slice.Write(Op::Return);
    std::ofstream out_file("test_1.mhm", std::ios::binary);

    const auto output = out_slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        hex::LogErr("Failed to write to file.");
        return;
    }

    out_file.close();
}

void ExecuteVM(const std::string_view exe_name) {
    namespace chrono = std::chrono;
    using namespace std::chrono_literals;

    const auto    start_file = chrono::steady_clock::now();
    std::ifstream in_file(std::string(exe_name), std::ios::binary);
    if (not in_file) {
        hex::LogErr("Failed to read file.");
        return;
    }
    in_file.seekg(0, std::ios::end);
    const auto file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<mana::literals::u8> raw(file_size);
    in_file.read(reinterpret_cast<char*>(raw.data()), file_size);

    const auto start_deser = chrono::steady_clock::now();
    hex::Slice in_slice;
    in_slice.Deserialize(raw);
    const auto end_deser = chrono::steady_clock::now();

    hex::Log("--- Reading executable '{}' ---", exe_name);
    hex::Log("");
    PrintBytecode(in_slice);
    hex::Log("");

    hex::Log("Executing...\n");
    hex::VirtualMachine vm;
    hex::SetLogPattern("%v");

    const auto start_interp = chrono::steady_clock::now();
    const auto interp_res   = vm.Interpret(&in_slice);
    const auto end_interp   = chrono::steady_clock::now();

    const auto result = magic_enum::enum_name(interp_res);
    hex::Log("");
    hex::SetLogPattern("%^<%n>%$ %v");
    hex::Log("Interpret Result: {}\n", result);

    const auto end_file = chrono::steady_clock::now();

    std::stringstream elapsed_file, elapsed_deser, elapsed_exec;

    elapsed_file << chrono::duration_cast<chrono::nanoseconds>(end_file - start_file);
    elapsed_deser << chrono::duration_cast<chrono::nanoseconds>(end_deser - start_deser);
    elapsed_exec << chrono::duration_cast<chrono::nanoseconds>(end_interp - start_interp);

    hex::
        Log("Elapsed time:\nTotal: {}\nDeserialize: {}\nExecute: {}",
            elapsed_file.str(),
            elapsed_deser.str(),
            elapsed_exec.str());
}

int main(const int argc, char** argv) {
    hex::CommandLineSettings cli(argc, argv);

    cli.Populate();

    if (cli.ShouldSayHi()) {
        hex::Log("Hiiiiiii :3c");
        hex::Log("");
    }

    if (cli.ShouldGenTestfile()) {
        hex::Log("Generating testfile...");
        WriteTestFile();
        return 0;
    }

    const std::string_view exe_name = cli.ExecutableName();
    if (exe_name.empty()) {
        return 0;
    }

    ExecuteVM(exe_name);
}