#include <salem/core/cli.hpp>
#include <salem/core/exit_codes.hpp>
#include <salem/core/logger.hpp>

namespace salem::cli {

commands::commands(const int argc, char** argv)
    : cli_(std::make_unique<CLI::App>("Salem, the Mana compiler.\n"))
    , argc_(argc)
    , argv_(argv) {
    cli_->add_option("-f,--file", options_.src_file, "Path to a Mana source file (.mn/.mana)")
        ->check(CLI::ExistingFile);

    cli_->add_flag("-v,--version", flags_.show_version, "Current Salem version");
    cli_->add_flag("-t,--tokens", flags_.print_tokens, "Print lexer tokens post-tokenization");
    cli_->add_flag("-a,--ast", flags_.print_ast, "Print AST post-parse");
    cli_->add_flag("-r,--repl", flags_.run_repl, "Run Mana REPL");
}

auto commands::process_args() const -> int {
    try {
        cli_->parse(argc_, argv_);
    } catch (const CLI::ParseError& e) {
        const auto cli_exit = cli_->exit(e);
        if (cli_exit == exit_code(exit::Success)) {
            const std::string helparg(argv_[1]);
            if (helparg == "--help" || helparg == "-h") {
                return exit_code(exit::CLI_HelpArgUsed);
            }
        }
        return cli_exit;
    }

    if (flags_.show_version) {
        log("{}", SALEM_VERSION_STR);
        return exit_code(exit::Success);
    }

    if (flags_.run_repl) {
        return exit_code(exit::Success);
    }

    if (options_.src_file == MANA_INVALID_SRC) {
        log(log_level::Error, "Missing source file.\nRun with --help for more information.");
        return exit_code(exit::CLI_MissingSrcFile);
    }

    log(log_level::Debug, "Source path: {}\n", options_.src_file);

    return exit_code(exit::Success);
}

auto commands::source_file() const -> std::string_view {
    return options_.src_file;
}

auto commands::requested_token_print() const -> bool {
    return flags_.print_tokens;
}

auto commands::requested_ast_print() const -> bool {
    return flags_.print_ast;
}

auto commands::requested_repl() const -> bool {
    return flags_.run_repl;
}

} // namespace salem::cli
