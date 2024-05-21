#include <Salem/Core/CLI.hpp>
#include <Salem/Core/ExitCodes.hpp>
#include <Salem/Core/Logger.hpp>

namespace salem::cli {

Settings::Settings(const int argc, char** argv)
    : cli_(std::make_unique<CLI::App>("Salem, the Mana compiler.\n"))
    , argc_(argc)
    , argv_(argv) {
    cli_->add_option("-f,--file", options_.src_file, "Path to a Mana source file (.mn/.mana)")
        ->check(CLI::ExistingFile);

    cli_->add_flag("-v,--version", flags_.show_version, "Current Salem version");

}

auto Settings::ProcessArgs() const -> int {
    try {
        cli_->parse(argc_, argv_);
    } catch (const CLI::ParseError& e) {
        const auto exit_code = cli_->exit(e);
        if (exit_code == EXIT::SUCCESS) {
            const std::string helparg(argv_[1]);
            if (helparg == "--help" || helparg == "-h") {
                return EXIT::CLI_HELP_ARG_USED;
            }
        }
        return exit_code;
    }

    if (flags_.show_version) {
        Log("{}", SALEM_VERSION_STR);
        return EXIT::SUCCESS;
    }

    if (options_.src_file == MANA_INVALID_SRC) {
        Log(LogLevel::Error, "Missing source file.\nRun with --help for more information.");
        return EXIT::CLI_MISSING_SRC_FILE;
    }

    Log(LogLevel::Debug, "Source path: {}", options_.src_file);

    return EXIT::SUCCESS;
}

auto Settings::SourceFile() const -> std::string_view {
    return options_.src_file;
}
} // namespace salem::cli
