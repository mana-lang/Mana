#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parser.hpp>

#include <mana/vm/slice.hpp>

#include <filesystem>
#include <fstream>

using namespace mana::literals;
using namespace circe;

constexpr std::string IN_PATH  = "assets/samples/";
constexpr std::string OUT_PATH = "../hex/";

int CreateFile(const std::string& filename) {
    Log->info("");

    sigil::Lexer lexer;

    const std::filesystem::path in = IN_PATH + filename;

    if (lexer.Tokenize(in)) {
        Log->info("Tokenized file from '{}'", in.string());
    }

    sigil::Parser parser(lexer.RelinquishTokens());
    if (parser.Parse()) {
        Log->info("Parsed the file.");
    }

    const auto& ast = parser.ViewAST();
    MainVisitor visitor;

    ast->Accept(visitor);

    mana::vm::Slice slice;
    slice = visitor.GetSlice();
    slice.Write(mana::vm::Op::Halt);

    std::filesystem::path out = OUT_PATH + in.filename().replace_extension("mhm").string();
    std::ofstream         out_file(out, std::ios::binary);
    Log->info("Output file to '{}'", out.string());

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), static_cast<std::streamsize>(output.size()));

    int ret = 0;
    if (not out_file) {
        Log->error("Failed to write to file.");
        ret = 3;
    }

    out_file.close();
    return ret;
}

int main() {
    using namespace circe;
    Log->info("Hello from Circe!");

    CreateFile("expr-a.mn");
    CreateFile("expr-b.mn");
    CreateFile("expr-c.mn");
}