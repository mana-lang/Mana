#include <Salem/Core/CLI.hpp>
#include <Salem/Core/ExitCodes.hpp>
#include <Salem/Core/Logger.hpp>

namespace salem::cli {

Interface::Interface(const int argc, char** argv)
    : cli_(std::make_unique<CLI::App>("Salem, the Mana compiler.\n"))
    , argc_(argc)
    , argv_(argv) {
    cli_->add_option("-f,--file", options_.src_file, "Path to a Mana source file (.mn/.mana)")
        ->check(CLI::ExistingFile);

    cli_->add_flag("-v,--version", flags_.show_version, "Current Salem version");
    cli_->add_flag("-t,--tokens", flags_.print_tokens, "Print lexer tokens post-tokenization");

}

auto Interface::ProcessArgs() const -> int {
    try {
        cli_->parse(argc_, argv_);
    } catch (const CLI::ParseError& e) {
        const auto exit_code = cli_->exit(e);
        if (exit_code == ExitCode(Exit::Success)) {
            const std::string helparg(argv_[1]);
            if (helparg == "--help" || helparg == "-h") {
                return ExitCode(Exit::CLI_HelpArgUsed);
            }
        }
        return exit_code;
    }

    if (flags_.show_version) {
        Log("{}", SALEM_VERSION_STR);
        return ExitCode(Exit::Success);
    }

    if (options_.src_file == MANA_INVALID_SRC) {
        Log(LogLevel::Error, "Missing source file.\nRun with --help for more information.");
        return ExitCode(Exit::CLI_MissingSrcFile);
    }

    Log(LogLevel::Debug, "Source path: {}\n", options_.src_file);

    return ExitCode(Exit::Success);
}

auto Interface::SourceFile() const -> std::string_view {
    return options_.src_file;
}

auto Interface::TokenPrintRequested() const -> bool {
    return flags_.print_tokens;
}

} // namespace salem::cli
