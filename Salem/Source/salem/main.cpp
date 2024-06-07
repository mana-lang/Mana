#include <salem/core/logger.hpp>
#include <salem/core/cli.hpp>
#include <salem/frontend/lexer.hpp>
#include <salem/frontend/repl.hpp>


/// TODO: make semicolons and newlines identical
/// This is well in line with Mana's philosophy of being like high and low level languages
/// TODO: figure out way to use concurrency for parsing/lexing
/// This can be achieved for parsing by
/// making its functions that define scopes (i.e. function declarations)
/// run as async, and only consuming them once the whole stream has been parsed
/// TODO: add variant keyword to design
/// Sum type for Mana
/// TODO: implement error sink
/// Error sink has its own analysis step
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