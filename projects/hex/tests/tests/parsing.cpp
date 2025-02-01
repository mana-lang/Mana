#include <catch2/catch_test_macros.hpp>

#include <hex/ast/lexer.hpp>
#include <hex/ast/parser.hpp>
#include "common.hpp"

constexpr auto PARSER_TESTING_PATH = "assets/samples/parsing/";

TEST_CASE("Parser", "[parse][ast]") {
    SECTION("Core", "Core functionality test") {
        hex::Lexer lexer;
        REQUIRE(lexer.Tokenize(Concatenate(PARSER_TESTING_PATH, "atoms.mn")));

        hex::Parser parser(lexer.RelinquishTokens());
        CHECK(parser.Parse());
        const auto& tokens {parser.ViewTokens()};
        const auto& ast {parser.ViewAST()};

        SECTION("Parsing step succeeded") {
            REQUIRE_FALSE(tokens.empty());
            REQUIRE(tokens[0].type == hex::TokenType::_module_);
        }

        using namespace hex::ast;
        SECTION("AST root is properly formed") {
            CHECK_FALSE(ast.tokens.empty());
            REQUIRE(ast.rule == Rule::Module);
            REQUIRE(ast.tokens[0].text == "atoms");
        }

        parser.PrintAST();

        SECTION("Primaries are correctly evaluated") {
            CHECK_FALSE(ast.branches.empty());

            hex::TokenStream expected_tokens {
                {
                    .type {hex::TokenType::Lit_Int},
                    .text {"27"},
                    .position {2, 1},
                },
                {
                    .type {hex::TokenType::Lit_Float},
                    .text {"7.27"},
                    .position {3, 1},
                },
                {
                    .type {hex::TokenType::Lit_String},
                    .text {"\"Blue\""},
                    .position {4, 1},
                },
                {
                    .type {hex::TokenType::Lit_Char},
                    .text {"'z'"},
                    .position {5, 1},
                },
                {
                    .type {hex::TokenType::Lit_true},
                    .text {"true"},
                    .position {6, 1},
                },
                {
                    .type {hex::TokenType::Lit_false},
                    .text {"false"},
                    .position {7, 1},
                },
                {
                    .type {hex::TokenType::Lit_null},
                    .text {"null"},
                    .position {8, 1},
                },
            };

            static constexpr hex::i64 total_literals = 7;
            for (int i = 0; i < total_literals; ++i) {
                REQUIRE(ast.branches[i]->rule == Rule::Primary);

                REQUIRE(ast.branches[i]->tokens.size() == 1);
                REQUIRE(ast.branches[i]->branches.size() == 0);

                REQUIRE(ast.branches[i]->tokens[0] == expected_tokens[i]);
            }
        }

        SECTION("Groupings") {
            REQUIRE(ast.branches.size() >= 10);

            const hex::TokenStream parens {
                // we don't check these against position
                {
                    .type {hex::TokenType::Op_ParenLeft},
                    .text {"("},
                },
                {
                    .type {hex::TokenType::Op_ParenRight},
                    .text {")"},
                },
            };

            const hex::TokenStream expected_tokens {
                {
                    .type {hex::TokenType::Lit_Int},
                    .text {"94"},
                    .position {16, 2},
                },
                {
                    .type {hex::TokenType::Lit_false},
                    .text {"false"},
                    .position {17, 2},
                },
                {
                    .type {hex::TokenType::Lit_Float},
                    .text {"34.853"},
                    .position {18, 2},
                },
            };

            using namespace hex;
            static constexpr i64 start {7};
            static constexpr i64 total_groupings {3};
            for (i64 i = 0; i < total_groupings; ++i) {
                const auto current {start + i};
                REQUIRE(ast.branches[current]->rule == Rule::Grouping);

                // LParen
                REQUIRE(ast.branches[current]->tokens[0].type == parens[0].type);
                REQUIRE(ast.branches[current]->tokens[0].text == parens[0].text);

                // RParen
                REQUIRE(ast.branches[current]->tokens[1].type == parens[1].type);
                REQUIRE(ast.branches[current]->tokens[1].text == parens[1].text);

                REQUIRE(
                    ast.branches[current]->branches[0]->tokens[0] == expected_tokens[i]
                );
            }
        }

        // SECTION("Unaries") {
        //     REQUIRE(ast.branches.size() >= 10);
        //
        //     hex::TokenStream expected_tokens {
        //         {
        //             .type     = hex::TokenType::Op_Minus,
        //             .text     = "-",
        //             .position = {11, 1},
        //         },
        //         {
        //             .type     = hex::TokenType::Lit_Int,
        //             .text     = "6",
        //             .position = {11, 2},
        //         },
        //
        //         {
        //             .type     = hex::TokenType::Op_Minus,
        //             .text     = "-",
        //             .position = {12, 1},
        //         },
        //         {
        //             .type     = hex::TokenType::Lit_Float,
        //             .text     = "45.795",
        //             .position = {12, 2},
        //         },
        //
        //         {
        //             .type     = hex::TokenType::Op_LogicalNot,
        //             .text     = "!",
        //             .position = {13, 1},
        //         },
        //         {
        //             .type     = hex::TokenType::Lit_true,
        //             .text     = "true",
        //             .position = {13, 2},
        //         },
        //     };
        //
        //     using namespace hex;
        //     static constexpr i64 Start = 7;
        //     static constexpr i64 TotalUnaries = 3;
        //     i64 ctr = 0;
        //     for (i64 i = 0; i < TotalUnaries; ++i) {
        //         REQUIRE(ast.branches[Start + i]->branches[0]->rule ==
        //         Rule::Unary); REQUIRE(ast.branches[Start +
        //         i]->branches[0]->tokens[0] == expected_tokens[i + ctr]);
        //         REQUIRE(ast.branches[Start
        //         + i]->branches[0]->tokens[1] == expected_tokens[i + ctr + 1]);
        //         ++ctr;
        //     }
        // }
    }
}