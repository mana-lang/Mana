#pragma once

#include <sigil/core/logger.hpp>

#include <string>
#include <fstream>
#include <filesystem>

namespace sigil {
// struct TokenizedSource {
//     std::string artifact;
//     std::string source;
//     TokenStream stream;
// };

// perhaps add some safeguards for parallelisation in the future
class GlobalSourceFile {
    friend class Lexer;

    std::string name;
    std::string contents;
    std::string_view view;
    uintmax_t size;

    GlobalSourceFile() = default;

    GlobalSourceFile(const GlobalSourceFile&) = delete;
    GlobalSourceFile& operator=(const GlobalSourceFile&) = delete;
    GlobalSourceFile(GlobalSourceFile&&) = delete;
    GlobalSourceFile& operator=(GlobalSourceFile&&) = delete;

private:
    bool Load(const std::filesystem::path& file_path) {
        std::ifstream source_file(file_path);
        if (not source_file.is_open()) {
            Log->error("Failed to open file at '{}'", file_path.string());
            return false;
        }

        size = std::filesystem::file_size(file_path);
        contents.resize(size);
        source_file.read(contents.data(), size);
        source_file.close();

        name = file_path.filename().replace_extension("").string();

        view = contents;

        return true;
    }

    void Reset() {
        contents.clear();
        name.clear();
    }

public:
    std::string_view Name() const {
        return name;
    }

    std::string_view Content() const {
        return contents;
    }

    std::string_view Slice(const std::size_t start, const std::size_t length) const {
        return view.substr(start, length);
    }

    uintmax_t Size() const {
        return size;
    }

    char operator[](const std::size_t index) const {
        return contents[index];
    }
};

} // namespace sigil
