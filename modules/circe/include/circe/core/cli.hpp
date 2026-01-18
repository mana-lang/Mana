#pragma once

#include <CLI11/CLI11.hpp>
#include <memory>
#include <string>
#include <filesystem>

namespace circe {
struct CompileSettings {
    friend CompileSettings ParseCommandLineCompileSettings(int argc, char** argv);

    CIRCE_NODISCARD const std::filesystem::path& InputFilePath() const;
    CIRCE_NODISCARD const std::filesystem::path& OutputPath() const;
    CIRCE_NODISCARD bool EmitVerbose() const;
    CIRCE_NODISCARD bool EmitParseTree() const;
    CIRCE_NODISCARD bool EmitTokens() const;

    CIRCE_NODISCARD int ErrorCode() const;

private:
    std::filesystem::path input_path;
    std::filesystem::path output_path;

    bool verbose;
    bool emit_ptree;
    bool emit_tokens;

    int exit_code;
};

CompileSettings ParseCommandLineCompileSettings(int argc, char** argv);
} // namespace circe
