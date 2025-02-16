#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parser.hpp>

#include <mana/vm/slice.hpp>

#include <fstream>

using namespace mana::literals;
using namespace circe;

constexpr auto FILE_TO_TOKENIZE = "assets/samples/expr-a.mn";

int main() {
    using namespace circe;
    Log->info("Hello from Circe!");

    sigil::Lexer lexer;
    if (lexer.Tokenize(FILE_TO_TOKENIZE)) {
        Log->info("Tokenized file from '{}'", FILE_TO_TOKENIZE);
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
    constexpr auto output_path = "../hex/test-circe.mhm";
    std::ofstream  out_file(output_path, std::ios::binary);
    Log->info("Output file to: {}", output_path);

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        Log->error("Failed to write to file.");
        return 3;
    }

    out_file.close();
}