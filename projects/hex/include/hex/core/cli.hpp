#pragma once

#include <CLI11/CLI11.hpp>
#include <mana/literals.hpp>

#include <memory>

namespace CLI {
class App;
}

namespace hex {
using namespace mana::literals;

struct CommandLineSettings {
    CommandLineSettings(int argc, char** argv);

    i64 Populate();

    HEX_NODISCARD bool ShouldSayHi() const;
    HEX_NODISCARD bool ShouldGenTestfile() const;
    HEX_NODISCARD auto ExecutableName() const -> std::string_view;

private:
    std::unique_ptr<CLI::App> cli;

    int    argc;
    char** argv;

    bool        say_hi;
    bool        gen_testfile;
    std::string executable;
};

}  // namespace hex