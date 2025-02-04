#include <catch2/catch_test_macros.hpp>

#include <hex/ast/lexer.hpp>
#include <hex/ast/parser.hpp>
#include "common.hpp"

constexpr auto PARSER_TESTING_PATH = "assets/samples/parsing/";
using namespace hex;

// remind me to never test like this again
TEST_CASE("Expression Parsing", "[parse][ast]") {
    SECTION("Core", "Core functionality test") {
        Lexer lexer;

        REQUIRE(lexer.Tokenize(Concatenate(PARSER_TESTING_PATH, "expressions.mn")));

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
            REQUIRE(ast.tokens[0].text == "expressions");
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

        SECTION("Terms") {
            REQUIRE(ast.branches.size() >= 20);

            SECTION("Two-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"63"},
                        .position {27, 1},
                    },
                    {
                        .type {TokenType::Op_Plus},
                        .text {"+"},
                        .position {27, 4},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"12"},
                        .position {27, 6},
                    },
                };

                // 63 + 12
                const auto& term = ast.branches[17];
                REQUIRE(term->rule == Rule::Term);
                REQUIRE(term->tokens.size() == 1);
                REQUIRE(term->tokens[0] == expected_tokens[1]);

                REQUIRE(term->branches.size() == 2);
                REQUIRE(term->branches[0]->tokens.size() == 1);
                REQUIRE(term->branches[1]->tokens.size() == 1);

                REQUIRE(term->branches[0]->tokens[0] == expected_tokens[0]);
                REQUIRE(term->branches[1]->tokens[0] == expected_tokens[2]);
            }

            SECTION("Multi-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"358"},
                        .position {28, 1},
                    },
                    {
                        .type {TokenType::Op_Minus},
                        .text {"-"},
                        .position {28, 5},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"54.91"},
                        .position {28, 7},
                    },
                    {
                        .type {TokenType::Op_Plus},
                        .text {"+"},
                        .position {28, 13},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"263.12"},
                        .position {28, 15},
                    },
                    {
                        .type {TokenType::Op_Minus},
                        .text {"-"},
                        .position {28, 22},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"958"},
                        .position {28, 24},
                    },
                    {
                        .type {TokenType::Op_Minus},
                        .text {"-"},
                        .position {28, 28},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"23"},
                        .position {28, 30},
                    },
                    {
                        .type {TokenType::Op_Plus},
                        .text {"+"},
                        .position {28, 33},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"6.37"},
                        .position {28, 35},
                    },
                };

                // 358 - 54.91 + 263.12 - 958 - 23 + 6.37
                const auto& factor = ast.branches[18];
                REQUIRE(factor->rule == Rule::Term);
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
                        .text {"97"},
                        .position {29, 1},
                    },
                    {
                        .type {TokenType::Op_Plus},
                        .text {"+"},
                        .position {29, 4},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"40"},
                        .position {29, 8},
                    },
                    {
                        .type {TokenType::Op_Minus},
                        .text {"-"},
                        .position {29, 11},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"17"},
                        .position {29, 13},
                    },
                    {
                        .type {TokenType::Op_Plus},
                        .text {"+"},
                        .position {29, 17},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"5.2"},
                        .position {29, 19},
                    },
                };

                // 97 + ((40 - 17) + 5.2)
                const auto& term = ast.branches[19];
                REQUIRE(term->rule == Rule::Term);
                REQUIRE(term->tokens.size() == 1);
                REQUIRE(term->branches.size() == 2);

                // 97
                REQUIRE(term->branches[0]->rule == Rule::Literal);
                CHECK(term->branches[0]->tokens[0] == expected_tokens[0]);

                // +
                CHECK(term->tokens[0] == expected_tokens[1]);

                // ((40 - 17) + 5.2)
                const auto& rhs = term->branches[1];
                REQUIRE(rhs->rule == Rule::Grouping);
                REQUIRE(rhs->branches.size() == 1);
                REQUIRE(rhs->tokens.size() == 2);
                CHECK(rhs->tokens[0].type == TokenType::Op_ParenLeft);
                CHECK(rhs->tokens[1].type == TokenType::Op_ParenRight);

                // (40 - 17) + 5.2
                const auto& inner_term = rhs->branches[0];
                REQUIRE(inner_term->rule == Rule::Term);
                CHECK(inner_term->tokens[0] == expected_tokens[5]);
                REQUIRE(inner_term->branches[0]->rule == Rule::Grouping);
                REQUIRE(inner_term->branches[0]->branches[0]->rule == Rule::Term);

                // 40 - 17
                const auto& innermost_term = inner_term->branches[0]->branches[0];

                REQUIRE(innermost_term->branches.size() == 2);
                for (i64 i = 0; i < 2; ++i) {
                    CHECK(innermost_term->branches[i]->rule == Rule::Literal);
                    CHECK(innermost_term->branches[i]->tokens[0] == expected_tokens[2 + i * 2]);
                }
            }
        }

        SECTION("Comparisons") {
            REQUIRE(ast.branches.size() >= 23);

            SECTION("Two-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"55"},
                        .position {32, 1},
                    },
                    {
                        .type {TokenType::Op_GreaterThan},
                        .text {">"},
                        .position {32, 4},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"23"},
                        .position {32, 6},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"72"},
                        .position {33, 1},
                    },
                    {
                        .type {TokenType::Op_GreaterEqual},
                        .text {">="},
                        .position {33, 4},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"125"},
                        .position {33, 7},
                    },
                };

                // 55 > 23
                const auto& cmp = ast.branches[20];
                REQUIRE(cmp->rule == Rule::Comparison);
                REQUIRE(cmp->tokens.size() == 1);
                REQUIRE(cmp->tokens[0] == expected_tokens[1]);

                REQUIRE(cmp->branches.size() == 2);
                REQUIRE(cmp->branches[0]->tokens.size() == 1);
                REQUIRE(cmp->branches[1]->tokens.size() == 1);

                REQUIRE(cmp->branches[0]->tokens[0] == expected_tokens[0]);
                REQUIRE(cmp->branches[1]->tokens[0] == expected_tokens[2]);

                // 72 >= 125
                const auto& cmp2 = ast.branches[21];
                REQUIRE(cmp2->rule == Rule::Comparison);
                REQUIRE(cmp2->tokens.size() == 1);
                REQUIRE(cmp2->tokens[0] == expected_tokens[4]);

                REQUIRE(cmp2->branches.size() == 2);
                REQUIRE(cmp2->branches[0]->tokens.size() == 1);
                REQUIRE(cmp2->branches[1]->tokens.size() == 1);

                REQUIRE(cmp2->branches[0]->tokens[0] == expected_tokens[3]);
                REQUIRE(cmp2->branches[1]->tokens[0] == expected_tokens[5]);
            }

            SECTION("Multi-operand expressions") {
                const TokenStream expected_tokens {
                    {
                        .type {TokenType::Lit_Int},
                        .text {"953"},
                        .position {34, 1},
                    },
                    {
                        .type {TokenType::Op_LessEqual},
                        .text {"<="},
                        .position {34, 5},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"24"},
                        .position {34, 8},
                    },
                    {
                        .type {TokenType::Op_LessThan},
                        .text {"<"},
                        .position {34, 11},
                    },
                    {
                        .type {TokenType::Lit_Int},
                        .text {"12"},
                        .position {34, 13},
                    },
                    {
                        .type {TokenType::Op_GreaterThan},
                        .text {">"},
                        .position {34, 16},
                    },
                    {
                        .type {TokenType::Lit_Float},
                        .text {"2384.5"},
                        .position {34, 18},
                    },
                    {
                        .type {TokenType::Op_GreaterEqual},
                        .text {">="},
                        .position {34, 25},
                    },
                    {
                        .type {TokenType::Lit_false},
                        .text {"false"},
                        .position {34, 28},
                    },
                };

                // 953 <= 24 < 12 > 2384.5 >= false
                const auto& cmp = ast.branches[22];
                REQUIRE(cmp->rule == Rule::Comparison);
                REQUIRE(cmp->tokens.size() == 4);
                REQUIRE(cmp->branches.size() == 5);

                for (i64 i = 1; const auto& token : cmp->tokens) {
                    CHECK(token == expected_tokens[i]);
                    i += 2;
                }

                for (i64 i = 0; const auto& branch : cmp->branches) {
                    CHECK(branch->tokens[0] == expected_tokens[i]);
                    i += 2;
                }
            }
        }

        SECTION("Equality") {
            REQUIRE(ast.branches.size() >= 25);

            const TokenStream expected_tokens {
                {
                    .type {TokenType::Lit_true},
                    .text {"true"},
                    .position {37, 1},
                },
                {
                    .type {TokenType::Op_Equality},
                    .text {"=="},
                    .position {37, 6},
                },
                {
                    .type {TokenType::Lit_false},
                    .text {"false"},
                    .position {37, 9},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"94"},
                    .position {38, 1},
                },
                {
                    .type {TokenType::Op_NotEqual},
                    .text {"!="},
                    .position {38, 4},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"94"},
                    .position {38, 7},
                },
            };

            // true == false
            const auto& equality = ast.branches[23];
            REQUIRE(equality->rule == Rule::Equality);
            REQUIRE(equality->tokens.size() == 1);
            REQUIRE(equality->tokens[0] == expected_tokens[1]);

            REQUIRE(equality->branches.size() == 2);
            REQUIRE(equality->branches[0]->tokens.size() == 1);
            REQUIRE(equality->branches[1]->tokens.size() == 1);

            REQUIRE(equality->branches[0]->tokens[0] == expected_tokens[0]);
            REQUIRE(equality->branches[1]->tokens[0] == expected_tokens[2]);

            // 94 != 94
            const auto& equality2 = ast.branches[24];
            REQUIRE(equality2->rule == Rule::Equality);
            REQUIRE(equality2->tokens.size() == 1);
            REQUIRE(equality2->tokens[0] == expected_tokens[4]);

            REQUIRE(equality2->branches.size() == 2);
            REQUIRE(equality2->branches[0]->tokens.size() == 1);
            REQUIRE(equality2->branches[1]->tokens.size() == 1);

            REQUIRE(equality2->branches[0]->tokens[0] == expected_tokens[3]);
            REQUIRE(equality2->branches[1]->tokens[0] == expected_tokens[5]);
        }

        SECTION("Multi-expression") {
            REQUIRE(ast.branches.size() >= 26);
            const TokenStream expected_tokens {
                {
                    .type {TokenType::Lit_Int},
                    .text {"83"},
                    .position {41, 1},
                },
                {
                    .type {TokenType::Op_Equality},
                    .text {"=="},
                    .position {41, 4},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"24"},
                    .position {41, 8},
                },
                {
                    .type {TokenType::Op_Plus},
                    .text {"+"},
                    .position {41, 11},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"94"},
                    .position {41, 13},
                },
                {
                    .type {TokenType::Op_FwdSlash},
                    .text {"/"},
                    .position {41, 16},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"3"},
                    .position {41, 19},
                },
                {
                    .type {TokenType::Op_Asterisk},
                    .text {"*"},
                    .position {41, 21},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"12"},
                    .position {41, 24},
                },
                {
                    .type {TokenType::Op_Minus},
                    .text {"-"},
                    .position {41, 27},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"34"},
                    .position {41, 29},
                },
                {
                    .type {TokenType::Op_LessThan},
                    .text {"<"},
                    .position {41, 34},
                },
                {
                    .type {TokenType::Lit_Float},
                    .text {"85.32"},
                    .position {41, 36},
                },
                {
                    .type {TokenType::Op_GreaterEqual},
                    .text {">="},
                    .position {41, 42},
                },
                {
                    .type {TokenType::Lit_Int},
                    .text {"120"},
                    .position {41, 45},
                },
                {
                    .type {TokenType::Op_NotEqual},
                    .text {"!="},
                    .position {41, 49},
                },
                {
                    .type {TokenType::Op_LogicalNot},
                    .text {"!"},
                    .position {41, 52},
                },
                {
                    .type {TokenType::Lit_false},
                    .text {"false"},
                    .position {41, 53},
                },
            };

            // 83 == -24 + 94 / (3 * (12 - 34)) < 85.32 >= 120 != !false;
            const auto& expr = ast.branches[25];
            REQUIRE(expr->rule == Rule::Equality);

            // eq -> lit | cmp | unary
            REQUIRE(expr->branches.size() == 3);

            // lit '==' cmp '!=' unary
            REQUIRE(expr->tokens.size() == 2);
            CHECK(expr->tokens[0] == expected_tokens[1]); // ==
            CHECK(expr->tokens[1] == expected_tokens[15]); // !=

            // 83
            REQUIRE(expr->branches[0]->rule == Rule::Literal);
            REQUIRE(expr->branches[0]->tokens[0] == expected_tokens[0]); // 83

            // -24 + 94 / (3 * (12 - 34)) < 85.32 >= 120
            REQUIRE(expr->branches[1]->rule == Rule::Comparison);
            const auto& cmp = expr->branches[1];
            REQUIRE(cmp->branches.size() == 3);

            // term '<' lit_float '>=' lit_int
            CHECK(cmp->tokens[0] == expected_tokens[11]); // <
            CHECK(cmp->tokens[1] == expected_tokens[13]); // >=

            // -24 + 94 / (3 * (12 - 34))
            REQUIRE(cmp->branches[0]->rule == Rule::Term);
            CHECK(cmp->branches[0]->tokens[0] == expected_tokens[3]); // +

            // -24
            REQUIRE(cmp->branches[0]->branches[0]->rule == Rule::Unary);
            CHECK(cmp->branches[0]->branches[0]->tokens[0].type == TokenType::Op_Minus);
            CHECK(cmp->branches[0]->branches[0]->branches[0]->tokens[0] == expected_tokens[2]); // 24

            // 94 / (3 * (12 - 34))
            REQUIRE(cmp->branches[0]->branches[1]->rule == Rule::Factor);
            const auto& outer_factor = cmp->branches[0]->branches[1];
            CHECK(outer_factor->tokens[0] == expected_tokens[5]); // /
            CHECK(outer_factor->branches[0]->tokens[0] == expected_tokens[4]); // 94

            // (3 * (12 - 34))
            REQUIRE(outer_factor->branches[1]->rule == Rule::Grouping);

            // 3 * (12 - 34)
            REQUIRE(outer_factor->branches[1]->branches[0]->rule == Rule::Factor);
            const auto& inner_factor = outer_factor->branches[1]->branches[0];
            REQUIRE(inner_factor->branches[1]->rule == Rule::Grouping);
            CHECK(inner_factor->branches[0]->tokens[0] == expected_tokens[6]); // 3
            CHECK(inner_factor->tokens[0] == expected_tokens[7]); // 3

            // 12 - 34
            REQUIRE(inner_factor->branches[1]->branches[0]->rule == Rule::Term);
            const auto& inner_term = inner_factor->branches[1]->branches[0];
            CHECK(inner_term->tokens[0] == expected_tokens[9]); // -
            CHECK(inner_term->branches[0]->tokens[0] == expected_tokens[8]); // 12
            CHECK(inner_term->branches[1]->tokens[0] == expected_tokens[10]); // 34

            // !false
            REQUIRE(expr->branches[2]->rule == Rule::Unary);
            CHECK(expr->branches[2]->tokens[0] == expected_tokens[16]); // !
            CHECK(expr->branches[2]->branches[0]->tokens[0] == expected_tokens[17]); // false
        }
    }
}
