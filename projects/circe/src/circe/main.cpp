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
    Log->debug("Hello from Circe!");

    sigil::Lexer lexer;
    if (lexer.Tokenize(FILE_TO_TOKENIZE)) {
        Log->debug("Tokenized file from '{}'", FILE_TO_TOKENIZE);
    }

    sigil::Parser parser(lexer.RelinquishTokens());
    if (parser.Parse()) {
        Log->debug("Parsed the file.");
    }

    mana::vm::Slice slice;
    const auto&     ast = parser.ViewAST();
    MainVisitor     visitor;

    ast->Accept(visitor);

    // for now
    slice = visitor.GetSlice();
    slice.Write(mana::vm::Op::Halt);
    constexpr auto output_path = "../hex/test-circe.mhm";
    std::ofstream  out_file(output_path, std::ios::binary);
    Log->debug("Output file to: {}", output_path);

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        Log->error("Failed to write to file.");
        return 3;
    }

    out_file.close();
}