#pragma once

#include <memory>

namespace CLI {
    class App;
}

namespace hex {
struct CLOptions {
    std::unique_ptr<CLI::App> cli;
};

} // namespace hex