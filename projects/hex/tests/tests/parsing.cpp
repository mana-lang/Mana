#include <catch2/catch_test_macros.hpp>

#include <hex/ast/parser.hpp>
#include <hex/ast/lexer.hpp>
#include "common.hpp"

constexpr auto PARSER_TESTING_PATH = "assets/samples/parsing/";
TEST_CASE("Parser", "[parse][ast]") {
    SECTION("Core", "Core functionality test") {
        hex::Lexer lexer;
        REQUIRE(lexer.Tokenize(
            Concatenate(PARSER_TESTING_PATH, "building-blocks.mn"))
        );

        hex::Parser parser(lexer.RelinquishTokens());
        CHECK(parser.Parse());
        const auto& tokens = parser.ViewTokens();
        const auto& ast_root = parser.ViewAST();

        SECTION("Parsing step succeeded") {
            REQUIRE_FALSE(tokens.empty());
            REQUIRE(tokens[0].type == hex::TokenType::_module_);
        }

        using namespace hex::ast;
        SECTION("AST root is properly formed") {
            CHECK_FALSE(ast_root.tokens.empty());
            REQUIRE(ast_root.rule == Rule::Module);
            REQUIRE(ast_root.tokens[0].text == "building-blocks");
        }

        SECTION("Terminal expressions are evaluated") {
            CHECK_FALSE(ast_root.branches.empty());

            const auto& branches {ast_root.branches};
            std::vector<std::string> expected_content {"27", "7.27", "\"Blue\"", "'z'"};
            for (int i = 0; i < branches.size(); ++i) {
                REQUIRE(branches[i]->rule == Rule::Expression);
                REQUIRE(branches[i]->tokens.size() == 0); // expr nodes have no tokens of their own
                REQUIRE(branches[i]->branches.size() == 1);

                REQUIRE(branches[i]->branches[0]->tokens[0].text == expected_content[i]);
            }
        }
    }
}