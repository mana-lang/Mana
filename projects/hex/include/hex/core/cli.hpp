#pragma once

#include <mana/type_aliases.hpp>

#include <memory>

namespace CLI {
class App;
}

namespace hex {
using namespace mana::aliases;
struct CommandLineSettings {
    CommandLineSettings();
    i64 populate(int argc, char** argv);

private:
    std::unique_ptr<CLI::App> cli;

    bool        say_hi;
    std::string some_opt;
};

}  // namespace hex