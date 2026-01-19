#include <circe/core/cli.hpp>

namespace circe {
const std::filesystem::path& CompileSettings::InputFilePath() const {
    return input_path;
}

const std::filesystem::path& CompileSettings::OutputPath() const {
    return output_path;
}

bool CompileSettings::EmitVerbose() const {
    return verbose;
}

bool CompileSettings::EmitParseTree() const {
    return emit_ptree;
}

bool CompileSettings::EmitTokens() const {
    return emit_tokens;
}

int CompileSettings::ErrorCode() const {
    return exit_code;
}

CompileSettings ParseCommandLineCompileSettings(int argc, char** argv) {
    auto cli = std::make_unique<CLI::App>("Circe, the Mana Bytecode Compiler");
    CompileSettings ret;

    cli->add_option("input", ret.input_path, "The Mana file to compile.")->required();

    cli->add_option("-o,--output,output",
                    ret.output_path,
                    "Path to output to. If left unspecified, Circe will output to the input folder."
    );

    cli->add_flag("-v,--verbose", ret.verbose, "Verbose output.");
    cli->add_flag("-p,--ptree", ret.emit_ptree, "Emit AST after compilation.");
    cli->add_flag("-t,--tokens", ret.emit_tokens, "Emit tokens after compilation.");

    try {
        cli->parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        ret.exit_code = cli->exit(e);
        return ret;
    }
    ret.exit_code = 0;
    return ret;
}
} // namespace circe
