#pragma once

#include <string>
#include <filesystem>

namespace sigil {
// perhaps add some safeguards for parallelisation in the future
class GlobalSourceFile {
    friend class Lexer;

    std::string name;
    std::string contents;
    std::size_t size = 0;

    std::string_view view;

    GlobalSourceFile() = default;

private:
    bool Load(const std::filesystem::path& file_path);
    void Reset();

public:
    SIGIL_NODISCARD std::size_t Size() const;

    SIGIL_NODISCARD std::string_view Name() const;
    SIGIL_NODISCARD std::string_view Content() const;
    SIGIL_NODISCARD std::string_view Slice(std::size_t start, std::size_t length) const;

    char operator[](std::size_t index) const;

public:
    GlobalSourceFile(const GlobalSourceFile&)            = delete;
    GlobalSourceFile& operator=(const GlobalSourceFile&) = delete;
    GlobalSourceFile(GlobalSourceFile&&)                 = delete;
    GlobalSourceFile& operator=(GlobalSourceFile&&)      = delete;
};

SIGIL_NODISCARD const GlobalSourceFile& Source();
SIGIL_NODISCARD std::string_view FetchTokenText(struct Token token);
} // namespace sigil
