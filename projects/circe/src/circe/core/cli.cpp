#include <circe/core/cli.hpp>

namespace circe {
CommandLineSettings::CommandLineSettings(int argc, char** argv)
    : argc {argc},
      argv {argv},
      verbose {false},
      emit_ptree {false},
      emit_tokens {false} {
    cli = std::make_unique<CLI::App>("Circe, the Mana Compiler");
}

int CommandLineSettings::Populate() {
    cli->add_option("input", input, "The Mana file to compile.")->required();

    cli->add_option("-o,--output,output",
                    output,
                    "Path to output to. If left unspecified, Circe will output to the input folder."
    );

    cli->add_flag("-v,--verbose", verbose, "Verbose output.");
    cli->add_flag("-p,--ptree", emit_ptree, "Emit AST after compilation.");
    cli->add_flag("-t,--tokens", emit_tokens, "Emit tokens after compilation.");

    try {
        cli->parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        return cli->exit(e);
    }
    return 0;
}

std::string CommandLineSettings::InputFile() const {
    return input;
}

std::string CommandLineSettings::OutputPath() const {
    return output;
}

bool CommandLineSettings::EmitVerbose() const {
    return verbose;
}

bool CommandLineSettings::EmitParseTree() const {
    return emit_ptree;
}

bool CommandLineSettings::EmitTokens() const {
    return emit_tokens;
}
}
