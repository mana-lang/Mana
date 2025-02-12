#include <circe/core/logger.hpp>
#include <circe/main-visitor.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/nodes.hpp>
#include <sigil/ast/parser.hpp>

#include <mana/vm/slice.hpp>

#include <fstream>

using namespace mana::literals;
using namespace circe;

int main() {
    using namespace circe;
    Log->debug("Hello from Circe!");

    sigil::Lexer lexer;
    if (lexer.Tokenize("assets/samples/expr-a.mn")) {
        Log->debug("Nice.");
    }

    sigil::Parser parser(lexer.RelinquishTokens());
    if (parser.Parse()) {
        Log->debug("Double nice.");
    }

    mana::vm::Slice slice;
    const auto&     ast = parser.ViewAST();
    MainVisitor     visitor;

    ast->Accept(visitor);

    // for now
    slice = visitor.GetSlice();
    slice.Write(mana::vm::Op::Halt);
    std::ofstream out_file("../hex/test-circe.mhm", std::ios::binary);

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        Log->error("Failed to write to file.");
        return 3;
    }

    out_file.close();
}