#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>

namespace hex {

CommandLineSettings::CommandLineSettings()
    : say_hi(false) {
    cli = std::make_unique<CLI::App>("Hex, the Mana VM");
}

i64 CommandLineSettings::Populate(const int argc, char** argv) {
    cli->add_flag("-g,--greet", say_hi, "Would you like me to greet you politely? Then set this flag.");
    cli->add_option("-s, --stuff", some_opt, "I don't know what this does, but it seems important.");

    try {
        cli->parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        const auto exit_code = cli->exit(e);
        if (exit_code == 0) {
            const std::string_view helparg(argv[1]);
            if (helparg == "--help" || helparg == "-h") {
                return 1;  // not an error code
            }
        }
        return exit_code;
    }
    return 0;
}

bool CommandLineSettings::ShouldSayHi() const {
    return say_hi;
}

auto CommandLineSettings::Opt() const -> std::string_view {
    return some_opt;
}
}  // namespace hex