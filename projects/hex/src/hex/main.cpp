#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <mana/vm/slice.hpp>

#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <fstream>

using namespace hex;

void ExecuteVM(const std::string_view exe_name) {
    namespace chrono = std::chrono;
    using namespace std::chrono_literals;

    const auto start_file = chrono::high_resolution_clock::now();

    std::ifstream in_file(std::string(exe_name), std::ios::binary);
    if (not in_file) {
        Log->error("Failed to read file '{}'", exe_name);
        return;
    }
    in_file.seekg(0, std::ios::end);
    const auto file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<u8> raw(file_size);
    in_file.read(reinterpret_cast<char*>(raw.data()), file_size);

    const auto start_deser = chrono::high_resolution_clock::now();
    mana::vm::Slice in_slice;
    in_slice.Deserialize(raw);
    const auto end_deser = chrono::high_resolution_clock::now();

    Log->debug("--- Reading executable '{}' ---", exe_name);
    Log->debug("");
    PrintBytecode(in_slice);
    Log->debug("");

    Log->debug("Executing...\n");
    VirtualMachine vm;
    Log->set_pattern("%v");

    const auto start_interp  = chrono::high_resolution_clock::now();
    const auto interp_result = vm.Interpret(&in_slice);
    const auto end_interp    = chrono::high_resolution_clock::now();

    const auto result = magic_enum::enum_name(interp_result);
    Log->debug("");
    Log->set_pattern("%^<%n>%$ %v");
    Log->debug("Interpret Result: {}\n", result);

    const auto end_file = chrono::high_resolution_clock::now();

    std::stringstream elapsed_file, elapsed_deser, elapsed_exec;

    elapsed_file << chrono::duration_cast<chrono::microseconds>(end_file - start_file);
    elapsed_deser << chrono::duration_cast<chrono::microseconds>(end_deser - start_deser);
    elapsed_exec << chrono::duration_cast<chrono::microseconds>(end_interp - start_interp);

    Log->debug(
        "Elapsed time:\nTotal: {}\nDeserialize: {}\nExecute: {}",
        elapsed_file.str(),
        elapsed_deser.str(),
        elapsed_exec.str()
    );
}

int main(const int argc, char** argv) {
    CommandLineSettings cli(argc, argv);

    cli.Populate();

    if (cli.ShouldSayHi()) {
        Log->debug("Hey!");
    }

    const std::string_view exe_name = cli.ExecutableName();
    if (exe_name.empty()) {
        return 0;
    }

    ExecuteVM(exe_name);
}
