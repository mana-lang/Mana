#include <catch2/catch_test_macros.hpp>

#include <sigil/ast/lexer.hpp>

#include <filesystem>

using namespace sigil;
namespace fs = std::filesystem;

TEST_CASE("Lexer", "[lex][token][operator][keyword]") {
    using enum TokenType;
    Lexer lexer;

    REQUIRE(lexer.Tokenize("assets/samples/lex-tests.mn"));
    lexer.PrintTokens(Lexer::PrintingMode::Emit, Lexer::PrintingPolicy::SkipTerminators);
    lexer.Reset();

}