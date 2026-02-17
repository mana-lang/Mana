#include <hex/core/cli.hpp>
#include <hex/core/disassembly.hpp>
#include <hex/core/logger.hpp>
#include <hex/hex.hpp>

#include <hexec/bytecode.hpp>

#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <fstream>

using namespace hex;
using namespace mana;

void Execute(const std::filesystem::path& hexe_path) {
    namespace chrono = std::chrono;
    using namespace std::chrono_literals;

    Log->debug("Hexe Bytecode Format Version: {}\n", hexec::Header::Version);

    const auto start_file = chrono::high_resolution_clock::now();

    std::ifstream in_file(hexe_path, std::ios::binary);
    if (not in_file) {
        Log->error("Failed to read file '{}'", hexe_path.c_str());
        return;
    }
    in_file.seekg(0, std::ios::end);
    const auto file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<u8> raw(file_size);
    in_file.read(reinterpret_cast<char*>(raw.data()), file_size);

    const auto start_deser = chrono::high_resolution_clock::now();
    hexec::ByteCode bytecode;
    bytecode.Deserialize(raw);
    const auto end_deser = chrono::high_resolution_clock::now();

    Log->debug("Entry point: {:08X}", bytecode.EntryPointValue());
    Log->debug("Main Register Frame: {}\n", bytecode.MainRegisterFrame());

    Log->debug("--- Reading executable '{}' ---", hexe_path.filename().c_str());
    Log->debug("");
    PrintBytecode(bytecode);

    Log->info("Executing...\n");
    Hex vm;

    const auto start_interp  = chrono::high_resolution_clock::now();
    const auto interp_result = vm.Execute(&bytecode);
    const auto end_interp    = chrono::high_resolution_clock::now();

    const auto result = magic_enum::enum_name(interp_result);
    Log->info("Interpret Result: {}\n", result);

    const auto end_file = chrono::high_resolution_clock::now();

    std::stringstream elapsed_file, elapsed_deser, elapsed_exec;

    elapsed_file << chrono::duration_cast<chrono::microseconds>(end_file - start_file);
    elapsed_deser << chrono::duration_cast<chrono::microseconds>(end_deser - start_deser);
    elapsed_exec << chrono::duration_cast<chrono::microseconds>(end_interp - start_interp);

    Log->info(
        "Elapsed time:\nTotal: {}\nDeserialize: {}\nExecute: {}",
        elapsed_file.str(),
        elapsed_deser.str(),
        elapsed_exec.str()
    );
}

int main(const int argc, char** argv) {
    CommandLineSettings cli(argc, argv);
    const i64 result = cli.Populate();

    if (cli.ShouldExit()) {
        return result;
    }

    const std::string_view hexe_name = cli.HexeName();
    if (hexe_name.empty()) {
        return result;
    }

    Execute(hexe_name);
}
