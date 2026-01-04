#include <catch2/catch_test_macros.hpp>

#include <sigil/ast/lexer.hpp>

#include <filesystem>
#include <fstream>

using namespace sigil;
namespace fs = std::filesystem;

TEST_CASE("Lexer", "[lex][token][operator][keyword]") {
    using enum TokenType;
    Lexer lexer;

    REQUIRE(lexer.Tokenize("assets/samples/lex-tests.mn"));
    lexer.PrintTokens(Lexer::PrintingMode::Emit, Lexer::PrintingPolicy::SkipTerminators);

    std::ifstream control_file("assets/control/lex-control.tks");
    REQUIRE(control_file.good());
    const std::string control(std::istreambuf_iterator{control_file}, {});

    std::ifstream output_file("lex-tests.tks");
    REQUIRE(output_file.good());
    const std::string output(std::istreambuf_iterator{output_file}, {});

    REQUIRE(output == control);
}