#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>

int main(const int argc, char** argv) {
    hex::CommandLineSettings cli;

    cli.Populate(argc, argv);

    if (cli.ShouldSayHi()) {
        hex::Log("Hiii :3c");
    }

    const std::string_view s = cli.Opt();
    if (not s.empty()) {
        hex::Log("I dunno what to do with {}, but it sure looks important!", s);
    }
}