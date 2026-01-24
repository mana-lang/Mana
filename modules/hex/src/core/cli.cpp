#include <hex/core/cli.hpp>

#include <mana/exit-codes.hpp>
#include <mana/literals.hpp>

namespace hex {
CommandLineSettings::CommandLineSettings(const int argc, char** argv)
    : argc(argc),
      argv(argv) {
    cli = std::make_unique<CLI::App>("Hex, the Mana VM");
}

i64 CommandLineSettings::Populate() {
    cli->add_option("-e, --executable,executable", hexe_name, "The executable to run.");

    try {
        cli->parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        const auto exit_code = cli->exit(e);
        if (exit_code == 0) {
            if (const std::string_view helparg(argv[1]);
                helparg == "--help" || helparg == "-h") {
                return mana::Exit(mana::ExitCode::Success);
            }
        }
        return exit_code;
    }
    return mana::Exit(mana::ExitCode::Success);
}

std::string_view CommandLineSettings::HexeName() const {
    return hexe_name;
}
} // namespace hex
