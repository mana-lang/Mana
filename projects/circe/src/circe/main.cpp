#include <circe/core/logger.hpp>

#include <sigil/ast/ast.hpp>
#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>

#include <mana/vm/slice.hpp>

#include <fstream>

class CirceVisitor final : public sigil::ast::Visitor {
    mana::vm::Slice slice;

public:
    void Visit(const sigil::ast::Module& node) override {
        for (const auto& child : node.GetChildren()) {
            child->Accept(*this);
            slice.Write(mana::vm::Op::Return);
        }
    }

    void Visit(const sigil::ast::BinaryOp& node) override {
        using namespace sigil::ast;
        using namespace mana::vm;

        node.GetLeft().Accept(*this);
        node.GetRight().Accept(*this);

        switch (node.GetOp()) {
        case '+':
            slice.Write(Op::Add);
            break;
        case '-':
            slice.Write(Op::Sub);
            break;
        case '*':
            slice.Write(Op::Mul);
            break;
        case '/':
            slice.Write(Op::Div);
            break;

        default:
            circe::LogErr("Unknown Binary Operator");
            break;
        }
    }

    void Visit(const sigil::ast::LiteralF64& node) override {
        slice.Write(mana::vm::Op::Constant, slice.AddConstant(node.Get()));
    }

    mana::vm::Slice GetSlice() const {
        return slice;
    }
};

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

    mana::vm::Slice slice;
    const auto&     ast = parser.ViewAST();
    CirceVisitor    visitor;

    ast->Accept(visitor);

    // for now
    slice = visitor.GetSlice();
    slice.Write(mana::vm::Op::Halt);
    std::ofstream out_file("../hex/test-circe.mhm", std::ios::binary);

    const auto output = slice.Serialize();
    out_file.write(reinterpret_cast<const char*>(output.data()), output.size());

    if (not out_file) {
        LogErr("Failed to write to file.");
        return 3;
    }

    out_file.close();
}