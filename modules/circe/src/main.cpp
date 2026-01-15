#include <circe/core/logger.hpp>
#include <circe/bytecode-generator.hpp>
#include <circe/core/cli.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>
#include <sigil/ast/syntax-tree.hpp>

#include <mana/vm/hexe.hpp>

#include <filesystem>
#include <fstream>

#include <mana/exit-codes.hpp>

using namespace mana::literals;
using namespace circe;

constexpr std::string IN_PATH  = "assets/samples/";
constexpr std::string OUT_PATH = "../hex/";

int CompileFrom(const std::filesystem::path& in_path,
                std::filesystem::path out_path,
                bool verbose,
                bool emit_ptree,
                bool emit_tokens
) {
    namespace chrono = std::chrono;
    using namespace mana;

    if (not std::filesystem::exists(in_path)) {
        Log->error("Input file '{}' does not exist", in_path.string());
        return Exit(ExitCode::FileNotFound);
    }

    const auto time_total_start = chrono::high_resolution_clock::now();

    const auto time_lex_start = chrono::high_resolution_clock::now();
    sigil::Lexer lexer;
    if (not lexer.Tokenize(in_path)) {
        Log->error("Failed to tokenize file '{}'", in_path.string());
        return Exit(ExitCode::LexerError);
    }
    const auto time_lex_end = chrono::high_resolution_clock::now();

    auto tokens            = lexer.RelinquishTokens();
    const auto token_count = tokens.size();

    const auto time_parse_start = chrono::high_resolution_clock::now();
    sigil::Parser parser(std::move(tokens));
    if (not parser.Parse()) {
        Log->error("Failed to parse file '{}'", in_path.string());
        return Exit(ExitCode::ParserError);
    }
    const auto time_parse_end = chrono::high_resolution_clock::now();

    const auto time_codegen_start = chrono::high_resolution_clock::now();
    const auto& ast               = parser.ViewAST();
    BytecodeGenerator generator;
    ast->Accept(generator);

    const auto hexe             = generator.GetBytecode();
    const auto time_codegen_end = chrono::high_resolution_clock::now();

    // If no output path is provided, use the input filename with .hexe extension
    if (out_path.empty()) {
        out_path = in_path;
        out_path.replace_extension("hexe");
    } else if (std::filesystem::is_directory(out_path) || not out_path.has_filename()) {
        out_path /= in_path.filename().replace_extension(".hexe");
    }

    if (out_path.has_parent_path()) {
        std::filesystem::create_directories(out_path.parent_path());
    }

    const auto time_write_start = chrono::high_resolution_clock::now();
    std::ofstream out_file(out_path, std::ios::binary);
    if (not out_file) {
        Log->error("Failed to open output file '{}'", out_path.string());
        return Exit(ExitCode::OutputOpenError);
    }

    const auto output = hexe.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), static_cast<std::streamsize>(output.size()));

    if (not out_file) {
        Log->error("Failed to write to output file '{}'", out_path.string());
        return Exit(ExitCode::OutputWriteError);
    }

    // write ends when total ends
    const auto time_total_end = chrono::high_resolution_clock::now();

    const auto time_lex     = chrono::duration_cast<chrono::microseconds>(time_lex_end - time_lex_start);
    const auto time_parse   = chrono::duration_cast<chrono::microseconds>(time_parse_end - time_parse_start);
    const auto time_codegen = chrono::duration_cast<chrono::microseconds>(time_codegen_end - time_codegen_start);
    const auto time_write   = chrono::duration_cast<chrono::microseconds>(time_total_end - time_write_start);
    const auto time_total   = chrono::duration_cast<chrono::microseconds>(time_total_end - time_total_start);

    const auto compile_str = fmt::format("Compiled '{}' => '{}'",
                                         in_path.filename().string(),
                                         out_path.filename().string()
    );

    Log->info(compile_str);
    Log->info("Operation completed in {}us", time_total.count());
    Log->info("Output written to '{}'", out_path.string());

    const auto divider = std::string(compile_str.size(), '-');

    if (verbose) {
        Log->info(divider);
        Log->info("  Tokens:         {}", token_count);
        Log->info("  Instructions:   {} bytes", hexe.Instructions().size());
        Log->info("  Constant Pool:  {} constants ({} bytes)", hexe.ConstantCount(), hexe.ConstantPoolBytesCount());
        Log->info("  Executable:     {} bytes", output.size());
        Log->info("");
        Log->info("  == Lex:     {}us", time_lex.count());
        Log->info("  == Parse:   {}us", time_parse.count());
        Log->info("  == Codegen: {}us", time_codegen.count());
        Log->info("  == Write:   {}us", time_write.count());
        Log->info("");
        Log->info("  ---- Total: {}us", time_total.count());
    }

    if (emit_ptree) {
        Log->info("{}\n", divider);
        parser.PrintParseTree();
    }

    if (emit_tokens) {
        Log->info("{}\n", divider);
        sigil::PrintTokens(parser.ViewTokenStream());
    }

    return Exit(ExitCode::Success);
}

int main(int argc, char** argv) {
    CommandLineSettings cli(argc, argv);
    if (int result = cli.Populate();
        result != 0) {
        return result;
    }

    if (cli.InputFile().empty()) {
        Log->error("No input file provided");
        return mana::Exit(mana::ExitCode::NoFileProvided);
    }

    return CompileFrom(cli.InputFile(), cli.OutputPath(), cli.EmitVerbose(), cli.EmitParseTree(), cli.EmitTokens());
}
