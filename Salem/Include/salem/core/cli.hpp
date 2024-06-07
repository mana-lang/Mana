#pragma once

#include <salem/core/type_aliases.hpp>
#include <CLI11/CLI11.hpp>

namespace salem::cli {
constexpr auto MANA_INVALID_SRC  = "##MANA_INVALID_SRC##";
constexpr auto SALEM_VERSION_STR = "0.0.1p";

struct options {
    std::string src_file = MANA_INVALID_SRC;
};

struct flags {
    bool show_version = false;
    bool print_tokens = false;
    bool print_ast    = false;
    bool run_repl     = false;
};

class commands {
    std::unique_ptr<CLI::App> cli_;
    options                   options_;
    flags                     flags_;

    const int argc_;
    char**    argv_;

public:
    commands(int argc, char** argv);

    SALEM_NODISCARD int  process_args() const;
    SALEM_NODISCARD bool requested_token_print() const;
    SALEM_NODISCARD bool requested_ast_print() const;
    SALEM_NODISCARD bool requested_repl() const;

    SALEM_NODISCARD auto source_file() const -> std::string_view;
};
} // namespace salem::cli
