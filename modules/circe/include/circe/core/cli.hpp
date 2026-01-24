#pragma once

#include <mana/literals.hpp>

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

    bool verbose {false};
    bool emit_ptree {false};
    bool emit_tokens {false};

    int exit_code {mana::literals::SENTINEL};
};

CompileSettings ParseCommandLineCompileSettings(int argc, char** argv);
} // namespace circe
