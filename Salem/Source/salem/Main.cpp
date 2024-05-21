#include <Salem/Core/Logger.hpp>
#include <Salem/Core/CLI.hpp>
#include <Salem/FrontEnd/Lexer.hpp>

int main(const int argc, char** argv) {
    using namespace salem;
    const cli::Interface cli(argc, argv);

    const int cli_status = cli.ProcessArgs();
    if (cli_status != ExitCode(Exit::Success)) {
        return cli_status;
    }

    const auto source_path = std::string(cli.SourceFile());

    Lexer lexer;
    if (not lexer.TokenizeFile(source_path)) {
        return ExitCode(Exit::Lexer_TokenizationFailed);
    }

    if (cli.TokenPrintRequested()) {
        lexer.PrintTokens();
    }
}