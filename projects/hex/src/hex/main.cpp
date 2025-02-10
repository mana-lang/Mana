#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/opcode.hpp>
#include <mana/vm/slice.hpp>

#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <fstream>

using namespace hex;

void WriteTestFile() {
    using namespace mana::vm;
    Slice out_slice;

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
        LogErr("Failed to write to file.");
        return;
    }

    out_file.close();
}

void ExecuteVM(const std::string_view exe_name) {
    namespace chrono = std::chrono;
    using namespace std::chrono_literals;

    const auto start_file = chrono::high_resolution_clock::now();

    std::ifstream in_file(std::string(exe_name), std::ios::binary);
    if (not in_file) {
        LogErr("Failed to read file.");
        return;
    }
    in_file.seekg(0, std::ios::end);
    const auto file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<mana::literals::u8> raw(file_size);
    in_file.read(reinterpret_cast<char*>(raw.data()), file_size);

    const auto      start_deser = chrono::high_resolution_clock::now();
    mana::vm::Slice in_slice;
    in_slice.Deserialize(raw);
    const auto end_deser = chrono::high_resolution_clock::now();

    Log("--- Reading executable '{}' ---", exe_name);
    Log("");
    PrintBytecode(in_slice);
    Log("");

    Log("Executing...\n");
    VirtualMachine vm;
    SetLogPattern("%v");

    const auto start_interp = chrono::high_resolution_clock::now();
    const auto interp_res   = vm.Interpret(&in_slice);
    const auto end_interp   = chrono::high_resolution_clock::now();

    const auto result = magic_enum::enum_name(interp_res);
    Log("");
    SetLogPattern("%^<%n>%$ %v");
    Log("Interpret Result: {}\n", result);

    const auto end_file = chrono::high_resolution_clock::now();

    std::stringstream elapsed_file, elapsed_deser, elapsed_exec;

    elapsed_file << chrono::duration_cast<chrono::microseconds>(end_file - start_file);
    elapsed_deser << chrono::duration_cast<chrono::microseconds>(end_deser - start_deser);
    elapsed_exec << chrono::duration_cast<chrono::microseconds>(end_interp - start_interp);

    Log("Elapsed time:\nTotal: {}\nDeserialize: {}\nExecute: {}",
        elapsed_file.str(),
        elapsed_deser.str(),
        elapsed_exec.str());
}

int main(const int argc, char** argv) {
    CommandLineSettings cli(argc, argv);

    cli.Populate();

    if (cli.ShouldSayHi()) {
        Log("Hiiiiiii :3c");
        Log("");
    }

    if (cli.ShouldGenTestfile()) {
        Log("Generating testfile...");
        WriteTestFile();
        return 0;
    }

    const std::string_view exe_name = cli.ExecutableName();
    if (exe_name.empty()) {
        return 0;
    }

    ExecuteVM(exe_name);
}