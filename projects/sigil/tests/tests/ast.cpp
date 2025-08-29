#include <catch2/catch_test_macros.hpp>

#include "headers/common.hpp"
#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>

constexpr auto PARSER_SAMPLE_PATH = "assets/samples/parsing/";
using namespace sigil;
using namespace mana::literals;

TEST_CASE("AST Construction", "[parse][ast]") {
    // SECTION("Core", "Core functionality test") {
    //     Lexer lexer;
    //
    //     REQUIRE(lexer.Tokenize(Concatenate(PARSER_TESTING_PATH, "ast.mn")));
    //
    //     Parser parser(lexer.RelinquishTokens());
    //     REQUIRE(parser.Parse());
    //
    //     const auto& tokens {parser.ViewTokens()};
    //     auto* const ast {parser.ViewAST()};
    //
    //     REQUIRE(ast != nullptr);
    // }
}