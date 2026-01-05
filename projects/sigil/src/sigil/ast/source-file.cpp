#include <sigil/ast/source-file.hpp>
#include <sigil/ast/lexer.hpp>
#include <sigil/core/logger.hpp>
#include <sigil/ast/token.hpp>

#include <fstream>

namespace ml = mana::literals;

namespace sigil {
bool GlobalSourceFile::Load(const std::filesystem::path& file_path) {
    std::ifstream source_file(file_path);
    if (not source_file.is_open()) {
        Log->error("Failed to open file at '{}'", file_path.string());
        return false;
    }

    size = std::filesystem::file_size(file_path);
    contents.resize(size);
    source_file.read(contents.data(), static_cast<ml::i64>(size));
    source_file.close();

    name = file_path.filename().replace_extension("").string();

    view = contents;

    return true;
}

void GlobalSourceFile::Reset() {
    contents.clear();
    name.clear();
    view = {};
    size = 0;
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

SIGIL_NODISCARD const GlobalSourceFile& Source() {
    return Lexer::Source;
}

SIGIL_NODISCARD std::string_view FetchTokenText(const Token token) {
    return Lexer::Source.Slice(token.offset, token.length);
}
} // namespace sigil
