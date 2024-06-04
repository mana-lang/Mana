#include <salem/core/logger.hpp>
#include <salem/core/cli.hpp>
#include <salem/frontend/lexer.hpp>
#include <salem/frontend/repl.hpp>

int main(const int argc, char** argv) {
    const salem::cli::commands commands(argc, argv);

    const int cli_status = commands.process_args();
    if (cli_status != exit_code(salem::exit::Success)) {
        return cli_status;
    }

    if (commands.requested_repl()) {
        salem::repl repl;
        repl.run();
        return 0;
    }

    const auto source_path = std::string(commands.source_file());

    salem::lexer lexer;
    if (not lexer.tokenize_file(source_path)) {
        return exit_code(salem::exit::LEX_TokenizationFailed);
    }

    if (commands.requested_token_print()) {
        lexer.print_tokens();
    }
}