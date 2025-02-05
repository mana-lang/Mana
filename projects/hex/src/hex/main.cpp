#include <hex/core/cli.hpp>
#include <hex/core/logger.hpp>

int main(const int argc, char** argv) {
    hex::CommandLineSettings cmline;

    cmline.Populate(argc, argv);

    if (cmline.ShouldSayHi()) {
        hex::Log("Hiii :3c");
    }

    const std::string_view s = cmline.Opt();
    if (not s.empty()) {
        hex::Log("I dunno what to do with {}, but it sure looks important!", s);
    }
}