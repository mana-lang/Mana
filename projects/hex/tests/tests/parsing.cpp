#include <catch2/catch_test_macros.hpp>

#include <hex/ast/lexer.hpp>
#include <hex/ast/parser.hpp>
#include "common.hpp"

constexpr auto PARSER_TESTING_PATH = "assets/samples/parsing/";
using namespace hex;

TEST_CASE("Parser", "[parse][ast]") {
    SECTION("Core", "Core functionality test") {
        Lexer lexer;

        REQUIRE(lexer.Tokenize(Concatenate(PARSER_TESTING_PATH, "atoms.mn")));

        Parser parser(lexer.RelinquishTokens());
        REQUIRE(parser.Parse());

        const auto& tokens {parser.ViewTokens()};
        const auto& ast {parser.ViewAST()};

        SECTION("Parsing step succeeded") {
            REQUIRE_FALSE(tokens.empty());
            REQUIRE(tokens[0].type == TokenType::_module_);
        }

        using namespace ast;
        SECTION("AST root is properly formed") {
            CHECK_FALSE(ast.tokens.empty());
            REQUIRE(ast.rule == Rule::Module);
            REQUIRE(ast.tokens[0].text == "atoms");
        }

        parser.PrintAST();

        SECTION("Primaries are correctly evaluated") {
            CHECK_FALSE(ast.branches.empty());

            TokenStream expected_tokens {
                {
                    .type {TokenType::Lit_Int},
                    .text {"27"},
                    .position {2, 1},
                },
                {
                    .type {TokenType::Lit_Float},
                    .text {"7.27"},
                    .position {3, 1},
                },
                {
                    .type {TokenType::Lit_String},
                    .text {"\"Blue\""},
                    .position {4, 1},
                },
                {
                    .type {TokenType::Lit_Char},
                    .text {"'z'"},
                    .position {5, 1},
                },
                {
                    .type {TokenType::Lit_true},
                    .text {"true"},
                    .position {6, 1},
                },
                {
                    .type {TokenType::Lit_false},
                    .text {"false"},
                    .position {7, 1},
                },
                {
                    .type {TokenType::Lit_null},
                    .text {"null"},
                    .position {8, 1},
                },
            };

            static constexpr i64 total_literals = 7;
            for (int i = 0; i < total_literals; ++i) {
                REQUIRE(ast.branches[i]->rule == Rule::Literal);

                REQUIRE(ast.branches[i]->tokens.size() == 1);
                REQUIRE(ast.branches[i]->branches.size() == 0);

                REQUIRE(ast.branches[i]->tokens[0] == expected_tokens[i]);
            }
        }

        SECTION("Unaries") {
            REQUIRE(ast.branches.size() >= 10);

            TokenStream expected_tokens {
                {
                    .type     = TokenType::Op_Minus,
                    .text     = "-",
                    .position = {11, 1},
                },
                {
                    .type     = TokenType::Lit_Int,
                    .text     = "6",
                    .position = {11, 2},
                },

                {
                    .type     = TokenType::Op_Minus,
                    .text     = "-",
                    .position = {12, 1},
                },
                {
                    .type     = TokenType::Lit_Float,
                    .text     = "45.795",
                    .position = {12, 2},
                },

                {
                    .type     = TokenType::Op_LogicalNot,
                    .text     = "!",
                    .position = {13, 1},
                },
                {
                    .type     = TokenType::Lit_true,
                    .text     = "true",
                    .position = {13, 2},
                },
            };

            constexpr i64 start {7};
            const usize   total_unaries {expected_tokens.size() / 2};

            for (i64 i = 0; i < total_unaries; ++i) {
                REQUIRE(ast.branches[start + i]->rule == Rule::Unary);
            }

            // "-6"
            REQUIRE(ast.branches[7]->tokens[0] == expected_tokens[0]);
            REQUIRE(ast.branches[7]->branches[0]->tokens[0] == expected_tokens[1]);

            // "-45.795"
            REQUIRE(ast.branches[8]->tokens[0] == expected_tokens[2]);
            REQUIRE(ast.branches[8]->branches[0]->tokens[0] == expected_tokens[3]);

            // "!true"
            REQUIRE(ast.branches[9]->tokens[0] == expected_tokens[4]);
            REQUIRE(ast.branches[9]->branches[0]->tokens[0] == expected_tokens[5]);
        }

        SECTION("Groupings") {
            REQUIRE(ast.branches.size() >= 14);

            const TokenStream parens {
                // we don't check these against position
                {
                    .type {TokenType::Op_ParenLeft},
                    .text {"("},
                },
                {
                    .type {TokenType::Op_ParenRight},
                    .text {")"},
                },
            };

            const TokenStream expected_tokens {
                {
                    .type {TokenType::Lit_Int},
                    .text {"94"},
                    .position {16, 2},
                },
                {
                    .type {TokenType::Lit_false},
                    .text {"false"},
                    .position {17, 2},
                },
                {
                    .type {TokenType::Lit_Float},
                    .text {"34.853"},
                    .position {18, 2},
                },
            };

            static constexpr i64 start {10};
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

                REQUIRE(ast.branches[current]->branches[0]->tokens[0] == expected_tokens[i]);
            }

            // !(!false)
            const auto& unary = ast.branches[13];
            REQUIRE(unary->rule == Rule::Unary);
            REQUIRE(unary->branches[0]->rule == Rule::Grouping);
            REQUIRE(unary->branches[0]->branches[0]->rule == Rule::Unary);
            REQUIRE(unary->branches[0]->branches[0]->branches[0]->rule == Rule::Literal);
        }

        SECTION("Factors") {
            REQUIRE(ast.branches.size() >= 17);

            SECTION("Two-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"26"},
                        .position {22, 1},
                    },
                    {
                        .type {TokenType::Op_Asterisk},
                        .text {"*"},
                        .position {22, 4},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"2"},
                        .position {22, 6},
                    },
                };

                // 26 * 2
                const auto& factor = ast.branches[14];
                REQUIRE(factor->rule == Rule::Factor);
                REQUIRE(factor->tokens.size() == 1);
                REQUIRE(factor->tokens[0] == expected_tokens[1]);

                REQUIRE(factor->branches.size() == 2);
                REQUIRE(factor->branches[0]->tokens.size() == 1);
                REQUIRE(factor->branches[1]->tokens.size() == 1);

                REQUIRE(factor->branches[0]->tokens[0] == expected_tokens[0]);
                REQUIRE(factor->branches[1]->tokens[0] == expected_tokens[2]);
            }

            SECTION("Multi-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"943"},
                        .position {23, 1},
                    },
                    {
                        .type {TokenType::Op_FwdSlash},
                        .text {"/"},
                        .position {23, 5},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"27.54"},
                        .position {23, 7},
                    },
                    {
                        .type {TokenType::Op_Asterisk},
                        .text {"*"},
                        .position {23, 13},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"12.95"},
                        .position {23, 15},
                    },
                    {
                        .type {TokenType::Op_FwdSlash},
                        .text {"/"},
                        .position {23, 21},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"599"},
                        .position {23, 23},
                    },
                    {
                        .type {TokenType::Op_FwdSlash},
                        .text {"/"},
                        .position {23, 27},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"2"},
                        .position {23, 29},
                    },
                    {
                        .type {TokenType::Op_Asterisk},
                        .text {"*"},
                        .position {23, 31},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"94.323"},
                        .position {23, 33},
                    },
                };

                // 943 / 27.54 * 12.95 / 599 / 2 * 94.323
                const auto& factor = ast.branches[15];
                REQUIRE(factor->rule == Rule::Factor);
                REQUIRE(factor->tokens.size() == 5);
                REQUIRE(factor->branches.size() == 6);

                for (i64 i = 1; const auto& token : factor->tokens) {
                    CHECK(token == expected_tokens[i]);
                    i += 2;
                }

                for (i64 i = 0; const auto& branch : factor->branches) {
                    CHECK(branch->tokens[0] == expected_tokens[i]);
                    i += 2;
                }
            }

            SECTION("Grouped expresssions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"5"},
                        .position {24, 1},
                    },
                    {
                        .type {TokenType::Op_Asterisk},
                        .text {"*"},
                        .position {24, 3},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"27"},
                        .position {24, 7},
                    },
                    {
                        .type {TokenType::Op_FwdSlash},
                        .text {"/"},
                        .position {24, 10},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"5"},
                        .position {24, 12},
                    },
                    {
                        .type {TokenType::Op_Asterisk},
                        .text {"*"},
                        .position {24, 15},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"2.5"},
                        .position {24, 17},
                    },
                };

                // 5 * ((27 / 5) * 2.5)
                const auto& factor = ast.branches[16];
                REQUIRE(factor->rule == Rule::Factor);
                REQUIRE(factor->tokens.size() == 1);
                REQUIRE(factor->branches.size() == 2);

                // 5
                REQUIRE(factor->branches[0]->rule == Rule::Literal);
                CHECK(factor->branches[0]->tokens[0] == expected_tokens[0]);

                // *
                CHECK(factor->tokens[0] == expected_tokens[1]);

                // ((27 / 5) * 2.5)
                const auto& rhs = factor->branches[1];
                REQUIRE(rhs->rule == Rule::Grouping);
                REQUIRE(rhs->branches.size() == 1);
                REQUIRE(rhs->tokens.size() == 2);
                CHECK(rhs->tokens[0].type == TokenType::Op_ParenLeft);
                CHECK(rhs->tokens[1].type == TokenType::Op_ParenRight);

                // (27 / 5) * 2.5
                const auto& inner_factor = rhs->branches[0];
                REQUIRE(inner_factor->rule == Rule::Factor);
                CHECK(inner_factor->tokens[0] == expected_tokens[5]);
                REQUIRE(inner_factor->branches[0]->rule == Rule::Grouping);
                REQUIRE(inner_factor->branches[0]->branches[0]->rule == Rule::Factor);

                // 27 / 5
                const auto& innermost_factor = inner_factor->branches[0]->branches[0];

                REQUIRE(innermost_factor->branches.size() == 2);
                for (i64 i = 0; i < 2; ++i) {
                    CHECK(innermost_factor->branches[i]->rule == Rule::Literal);
                    CHECK(innermost_factor->branches[i]->tokens[0] == expected_tokens[2 + i * 2]);
                }
            }
        }
    }
}