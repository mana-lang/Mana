#pragma once

#include <CLI11/CLI11.hpp>
#include <memory>
#include <string>

namespace circe {
struct CommandLineSettings {
    CommandLineSettings(int argc, char** argv);

    int Populate();

    CIRCE_NODISCARD std::string InputFile() const;
    CIRCE_NODISCARD std::string OutputFile() const;
    CIRCE_NODISCARD bool EmitVerbose() const;
    CIRCE_NODISCARD bool EmitParseTree() const;
    CIRCE_NODISCARD bool EmitTokens() const;

private:
    std::unique_ptr<CLI::App> cli;

    int    argc;
    char** argv;

    std::string input;
    std::string output;

    bool verbose;
    bool emit_ptree;
    bool emit_tokens;
};
}
