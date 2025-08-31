#pragma once

#include <sigil/core/logger.hpp>

#include <string>
#include <fstream>
#include <filesystem>

namespace sigil {
// perhaps add some safeguards for parallelisation in the future
class GlobalSourceFile {
    friend class Lexer;

    std::string name;
    std::string contents;
    std::size_t size;

    std::string_view view;

    GlobalSourceFile() = default;
    GlobalSourceFile(const GlobalSourceFile&)            = delete;
    GlobalSourceFile& operator=(const GlobalSourceFile&) = delete;
    GlobalSourceFile(GlobalSourceFile&&)                 = delete;
    GlobalSourceFile& operator=(GlobalSourceFile&&)      = delete;

private:
    bool Load(const std::filesystem::path& file_path);
    void Reset();

public:
    std::size_t Size() const;

    std::string_view Name() const;
    std::string_view Content() const;
    std::string_view Slice(std::size_t start, std::size_t length) const;

    char operator[](std::size_t index) const;
};
} // namespace sigil
