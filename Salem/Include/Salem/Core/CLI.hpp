#pragma once

#include <Salem/Core/TypeAliases.hpp>
#include <CLI11/CLI11.hpp>

namespace salem::cli {
constexpr auto MANA_INVALID_SRC = "##MANA_INVALID_SRC##";
constexpr auto SALEM_VERSION_STR = "0.0.1p";

    struct Options {
        std::string src_file = MANA_INVALID_SRC;
    };

    struct Flags {
        bool show_version = false;
    };

    class Settings {
        std::unique_ptr<CLI::App> cli_;
        Options options_;
        Flags flags_;

        const int argc_;
        const CString* argv_;

    public:
        Settings(int argc, char** argv);
        SALEM_NODISCARD auto ProcessArgs() const -> int;
        SALEM_NODISCARD auto SourceFile() const -> std::string_view;


    };

} // namespace salem::cli