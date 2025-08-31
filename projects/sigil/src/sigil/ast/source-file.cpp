#include <sigil/ast/source-file.hpp>

namespace sigil {
bool GlobalSourceFile::Load(const std::filesystem::path& file_path) {
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

void GlobalSourceFile::Reset() {
    contents.clear();
    name.clear();
}

std::string_view GlobalSourceFile::Name() const {
    return name;
}

std::string_view GlobalSourceFile::Content() const {
    return contents;
}

std::string_view GlobalSourceFile::Slice(const std::size_t start, const std::size_t length) const {
    return view.substr(start, length);
}

uintmax_t GlobalSourceFile::Size() const {
    return size;
}

char GlobalSourceFile::operator[](const std::size_t index) const {
    return contents[index];
}
} // namespace sigil
