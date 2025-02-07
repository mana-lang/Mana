#include <hex/core/cli.hpp>

#include <mana/literals.hpp>

namespace hex {
CommandLineSettings::CommandLineSettings(const int argc, char** argv)
    : argc(argc)
    , argv(argv)
    , say_hi(false)
    , gen_testfile(false) {
    cli = std::make_unique<CLI::App>("Hex, the Mana VM");
}

i64 CommandLineSettings::Populate() {
    cli->add_flag("-g,--greet", say_hi, "A polite greeting.");
    cli->add_flag("-t", gen_testfile, "Generate a testfile.");
    cli->add_option("-e, --executable", executable, "The executable to run.");

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

bool CommandLineSettings::ShouldGenTestfile() const {
    return gen_testfile;
}

auto CommandLineSettings::ExecutableName() const -> std::string_view {
    return executable;
}
}  // namespace hex