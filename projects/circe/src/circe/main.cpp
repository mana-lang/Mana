#include <circe/core/logger.hpp>

#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>

#include <hex/vm/slice.hpp>

#include <fstream>

int main() {
    using namespace circe;
    Log("Hello from Circe!");

    sigil::Lexer lexer;
    if (lexer.Tokenize("assets/samples/expr-a.mn")) {
        Log("Nice.");
    }

    sigil::Parser parser(lexer.RelinquishTokens());
    if (parser.Parse()) {
        Log("Double nice.");
    }

    hex::Slice  slice;
    const auto& ast = parser.ViewAST();

    for (const auto& branch : ast.branches) {
        using namespace mana::vm;
        if (branch->rule == sigil::ast::Rule::Term) {
            const auto a = slice.AddConstant(std::stod(branch->branches[0]->tokens[0].text));
            const auto b = slice.AddConstant(std::stod(branch->branches[1]->tokens[0].text));
            slice.Write(Op::Constant, a);
            slice.Write(Op::Constant, b);
            slice.Write(Op::Add);
        }
    }

    // for now
    slice.Write(mana::vm::Op::Return);

    std::ofstream out_file("test-circe.mhm", std::ios::binary);

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        LogErr("Failed to write to file.");
        return 3;
    }

    out_file.close();
}