#include <Salem/Core/Logger.hpp>
#include <Salem/Core/CLI.hpp>

int main(const int argc, char** argv) {
    const salem::cli::Settings cli(argc, argv);

    const int cli_status = cli.ProcessArgs();
    if (cli_status != salem::EXIT::SUCCESS) {
        return cli_status;
    }

    const auto source_path = std::string(cli.SourceFile());
}