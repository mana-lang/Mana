#include <circe/core/cli.hpp>

#include <CLI11/CLI11.hpp>

namespace circe {
const std::filesystem::path& CompileSettings::InputFilePath() const {
    return input_path;
}

const std::filesystem::path& CompileSettings::OutputPath() const {
    return output_path;
}

bool CompileSettings::EmitVerbose() const {
    return emit_detail;
}

bool CompileSettings::EmitParseTree() const {
    return emit_ptree;
}

bool CompileSettings::EmitTokens() const {
    return emit_tokens;
}

bool CompileSettings::ShouldExit() const {
    return should_exit;
}

int CompileSettings::ErrorCode() const {
    return exit_code;
}

CompileSettings ParseCommandLineCompileSettings(int argc, char** argv) {
    auto cli = std::make_unique<CLI::App>("Circe, the Mana Bytecode Compiler");
    CompileSettings ret;

    cli->set_version_flag("-v,--version", "Sigil v" SIGIL_VER_STRING "\nCirce v" CIRCE_VER_STRING);
    cli->add_option("input", ret.input_path, "The Mana file to compile.")->required();

    cli->add_option("-o,--output,output",
                    ret.output_path,
                    "Path to output to. If left unspecified, Circe will output to the input folder."
    );

    cli->add_flag("-d,--detailed", ret.emit_detail, "Detailed output.");
    cli->add_flag("-p,--ptree", ret.emit_ptree, "Emit AST after compilation.");
    cli->add_flag("-t,--tokens", ret.emit_tokens, "Emit tokens after compilation.");

    ret.exit_code = 0;
    try {
        cli->parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        ret.exit_code   = cli->exit(e);
        ret.should_exit = true;
    }
    return ret;
}
} // namespace circe
