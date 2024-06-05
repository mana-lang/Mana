#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <salem/frontend/lexer.hpp>
#include <salem/frontend/parser.hpp>
#include "common.hpp"


constexpr auto PARSER_TESTING_PATH = "../lang/samples/";
TEST_CASE("Parser", "[parse][ast]") {
    SECTION("MFT") {
        salem::lexer lexer;
        REQUIRE(lexer.tokenize_file(
            concatenate(PARSER_TESTING_PATH, "00-building-blocks.mn"))
        );

        salem::parser parser(lexer.relinquish_tokens());
        CHECK(parser.parse());
        const auto& tokens = parser.view_tokens();
        const auto& ast_root = parser.view_ast();

        SECTION("Parsing step succeeded") {
            REQUIRE_FALSE(tokens.empty());
            REQUIRE(tokens[0].type_ == salem::token_type::_module_);
        }

        using namespace salem::ast;
        SECTION("AST root is properly formed") {
            CHECK_FALSE(ast_root.tokens_.empty());
            CHECK(ast_root.subnodes_.empty());
            REQUIRE(ast_root.rule_ == rule::Module);
            REQUIRE(ast_root.tokens_[0].text_ == "00-building-blocks");
        }
    }


}