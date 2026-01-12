#include <catch2/catch_test_macros.hpp>

#include "headers/common.hpp"
#include <sigil/ast/lexer.hpp>
#include <sigil/ast/parser.hpp>

#include <filesystem>
#include <fstream>
#include <spdlog/fmt/bundled/chrono.h>

constexpr auto PARSER_SAMPLE_PATH  = "assets/samples/parsing/";
constexpr auto PARSER_CONTROL_PATH = "assets/control/ptree/";

using namespace sigil;
using namespace sigil::ast;
using namespace mana::literals;

TEST_CASE("P-Trees", "[parse][ast]") {
    SECTION("Expressions", "P-Tree for expression parsing gets properly constructed") {
        Lexer lexer;

        const auto path = Concatenate(PARSER_SAMPLE_PATH, "expressions.mn");
        REQUIRE(lexer.Tokenize(path));

        Parser parser(lexer.RelinquishTokens());

        REQUIRE(parser.Parse());

        const auto& tokens = parser.ViewTokenStream();
        const auto& ptree  = parser.ViewParseTree();

        SECTION("Root is properly formed") {
            REQUIRE_FALSE(tokens.empty());
            REQUIRE(ptree.rule == Rule::Artifact);
            REQUIRE_FALSE(ptree.branches.empty());
        }

        SECTION("P-Tree output matches control") {
            std::ifstream file(Concatenate(PARSER_CONTROL_PATH, "expressions.ptree"));
            REQUIRE((file && file.is_open()));

            const std::string file_str(std::istreambuf_iterator {file}, {});
            const std::string ptree_str = parser.EmitParseTree();

            REQUIRE(file_str == ptree_str);
        }

        parser.PrintParseTree();
    }

    auto compare_ptree = [](const std::string_view module_name) {
        const auto path = Concatenate(Concatenate(PARSER_SAMPLE_PATH, module_name), ".mn");

        Lexer lexer;
        REQUIRE(lexer.Tokenize(path));

        Parser parser(lexer.RelinquishTokens());
        REQUIRE(parser.Parse());

        std::ifstream file(Concatenate(Concatenate(PARSER_CONTROL_PATH, module_name), ".ptree"));
        REQUIRE((file && file.is_open()));

        const std::string file_str(std::istreambuf_iterator {file}, {});
        const std::string ptree_str = parser.EmitParseTree();
        parser.PrintParseTree();

        REQUIRE(file_str == ptree_str);
    };

    SECTION("Arrays") {
        compare_ptree("arrays");
    }

    SECTION("Conditionals") {
        compare_ptree("if");
    }
}
