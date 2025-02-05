#pragma once

#include <CLI11/CLI11.hpp>
#include <mana/type_aliases.hpp>
#include <memory>

namespace CLI {
class App;
}

namespace hex {
using namespace mana::aliases;

struct CommandLineSettings {
    CommandLineSettings();

    i64                Populate(int argc, char** argv);
    HEX_NODISCARD bool ShouldSayHi() const;
    HEX_NODISCARD auto Opt() const -> std::string_view;

private:
    std::unique_ptr<CLI::App> cli;

    bool        say_hi;
    std::string some_opt;
};

}  // namespace hex