#include <catch2/catch_test_macros.hpp>

#include "headers/common.hpp"
#include <sigil/ast/lexer.hpp>

constexpr auto LEXER_TESTING_PATH = "assets/samples/lexing/";

TEST_CASE("Lexer", "[lex][token][operator][keyword]") {
    using enum sigil::TokenType;

    SECTION("Core", "Core functionality test") {
        sigil::Lexer lexer;
        REQUIRE(lexer.Tokenize(Concatenate(LEXER_TESTING_PATH, "basic.mn")));

        const auto tokens = StripRedundant(lexer.RelinquishTokens());
        REQUIRE(tokens.size() == 16);

        SECTION("Tokens are being processed") {
            CHECK(tokens[0].type == KW_data);
            CHECK(tokens[0].text == "data");

            CHECK(tokens[1].type == Identifier);
            CHECK(tokens[1].text == "x");

            CHECK(tokens[2].type == Op_Assign);
            CHECK(tokens[2].text == "=");

            CHECK(tokens[3].type == Lit_Int);
            CHECK(tokens[3].text == "5");
        }

        SECTION("Tokens are correctly registered") {
            CHECK(tokens[4].type == Lit_Float);
            CHECK(tokens[4].text == "10.783");

            CHECK(tokens[5].type == KW_data);
            CHECK(tokens[5].text == "data");

            CHECK(tokens[6].type == Identifier);
            CHECK(tokens[6].text == "add");

            CHECK(tokens[7].type == Op_Assign);
            CHECK(tokens[7].text == "=");

            CHECK(tokens[8].type == Op_ParenLeft);
            CHECK(tokens[8].text == "(");

            CHECK(tokens[9].type == Op_ParenRight);
            CHECK(tokens[9].text == ")");

            CHECK(tokens[10].type == Op_BraceLeft);
            CHECK(tokens[10].text == "{");

            CHECK(tokens[11].type == Identifier);
            CHECK(tokens[11].text == "x");

            CHECK(tokens[12].type == Op_Plus);
            CHECK(tokens[12].text == "+");

            CHECK(tokens[13].type == Identifier);
            CHECK(tokens[13].text == "y");

            CHECK(tokens[14].type == Op_BraceRight);
            CHECK(tokens[14].text == "}");

            CHECK(tokens[15].type == Eof);
            CHECK(tokens[15].text == "EOF");
        }
    }

    const auto keyword_path = Concatenate(LEXER_TESTING_PATH, "keywords/");
    SECTION("Keyword lexing") {
        sigil::Lexer lexer;
        REQUIRE(lexer.Tokenize(Concatenate(keyword_path, "datatypes.mn")));

        const auto datatypetokens = StripRedundant(lexer.RelinquishTokens());
        REQUIRE(datatypetokens.size() == 20);

        SECTION("Primitive Datatypes") {
            SECTION("Integers") {
                CHECK(datatypetokens[0].type == KW_i8);
                CHECK(datatypetokens[0].text == "i8");

                CHECK(datatypetokens[1].type == KW_i16);
                CHECK(datatypetokens[1].text == "i16");

                CHECK(datatypetokens[2].type == KW_i32);
                CHECK(datatypetokens[2].text == "i32");

                CHECK(datatypetokens[3].type == KW_i64);
                CHECK(datatypetokens[3].text == "i64");

                CHECK(datatypetokens[4].type == KW_i128);
                CHECK(datatypetokens[4].text == "i128");

                CHECK(datatypetokens[5].type == KW_u8);
                CHECK(datatypetokens[5].text == "u8");

                CHECK(datatypetokens[6].type == KW_u16);
                CHECK(datatypetokens[6].text == "u16");

                CHECK(datatypetokens[7].type == KW_u32);
                CHECK(datatypetokens[7].text == "u32");

                CHECK(datatypetokens[8].type == KW_u64);
                CHECK(datatypetokens[8].text == "u64");

                CHECK(datatypetokens[9].type == KW_u128);
                CHECK(datatypetokens[9].text == "u128");

                CHECK(datatypetokens[12].type == KW_byte);
                CHECK(datatypetokens[12].text == "byte");
            }

            SECTION("Floats") {
                CHECK(datatypetokens[10].type == KW_f32);
                CHECK(datatypetokens[10].text == "f32");

                CHECK(datatypetokens[11].type == KW_f64);
                CHECK(datatypetokens[11].text == "f64");
            }

            SECTION("Textual Types") {
                CHECK(datatypetokens[13].type == KW_char);
                CHECK(datatypetokens[13].text == "char");

                CHECK(datatypetokens[14].type == KW_string);
                CHECK(datatypetokens[14].text == "string");
            }

            SECTION("Misc") {
                CHECK(datatypetokens[15].type == KW_bool);
                CHECK(datatypetokens[15].text == "bool");

                CHECK(datatypetokens[16].type == Lit_null);
                CHECK(datatypetokens[16].text == "null");

                CHECK(datatypetokens[17].type == Op_ParenLeft);
                CHECK(datatypetokens[17].text == "(");

                CHECK(datatypetokens[18].type == Op_ParenRight);
                CHECK(datatypetokens[18].text == ")");
            }
        }

        REQUIRE(lexer.Tokenize(Concatenate(keyword_path, "declarations.mn")));

        const auto declaration_tokens = StripRedundant(lexer.RelinquishTokens());
        REQUIRE(declaration_tokens.size() == 16);

        SECTION("Declarations") {
            SECTION("Data Declarators") {
                CHECK(declaration_tokens[0].type == KW_data);
                CHECK(declaration_tokens[0].text == "data");

                CHECK(declaration_tokens[1].type == KW_fn);
                CHECK(declaration_tokens[1].text == "fn");

                CHECK(declaration_tokens[2].type == KW_mut);
                CHECK(declaration_tokens[2].text == "mut");

                CHECK(declaration_tokens[3].type == KW_const);
                CHECK(declaration_tokens[3].text == "const");

                CHECK(declaration_tokens[4].type == KW_raw);
                CHECK(declaration_tokens[4].text == "raw");

                CHECK(declaration_tokens[13].type == KW_override);
                CHECK(declaration_tokens[13].text == "override");
            }

            SECTION("UDT Declarators") {
                CHECK(declaration_tokens[5].type == KW_pack);
                CHECK(declaration_tokens[5].text == "pack");

                CHECK(declaration_tokens[6].type == KW_struct);
                CHECK(declaration_tokens[6].text == "struct");

                CHECK(declaration_tokens[7].type == KW_enum);
                CHECK(declaration_tokens[7].text == "enum");

                CHECK(declaration_tokens[12].type == KW_generic);
                CHECK(declaration_tokens[12].text == "generic");
            }

            SECTION("Module Declarators") {
                CHECK(declaration_tokens[8].type == KW_artifact);
                CHECK(declaration_tokens[8].text == "module");

                CHECK(declaration_tokens[9].type == KW_public);
                CHECK(declaration_tokens[9].text == "public");

                CHECK(declaration_tokens[10].type == KW_private);
                CHECK(declaration_tokens[10].text == "private");

                CHECK(declaration_tokens[11].type == KW_import);
                CHECK(declaration_tokens[11].text == "import");

                CHECK(declaration_tokens[14].type == KW_as);
                CHECK(declaration_tokens[14].text == "as");
            }
        }

        REQUIRE(lexer.Tokenize(Concatenate(keyword_path, "control_flow.mn")));

        const auto controlflow_tokens = StripRedundant(lexer.RelinquishTokens());
        REQUIRE(controlflow_tokens.size() == 12);

        SECTION("Control Flow") {
            SECTION("Branching") {
                CHECK(controlflow_tokens[0].type == KW_return);
                CHECK(controlflow_tokens[0].text == "return");

                CHECK(controlflow_tokens[1].type == Lit_true);
                CHECK(controlflow_tokens[1].text == "true");

                CHECK(controlflow_tokens[2].type == Lit_false);
                CHECK(controlflow_tokens[2].text == "false");

                CHECK(controlflow_tokens[3].type == KW_if);
                CHECK(controlflow_tokens[3].text == "if");

                CHECK(controlflow_tokens[4].type == KW_else);
                CHECK(controlflow_tokens[4].text == "else");

                CHECK(controlflow_tokens[5].type == KW_match);
                CHECK(controlflow_tokens[5].text == "match");
            }

            SECTION("Iteration") {
                CHECK(controlflow_tokens[6].type == KW_loop);
                CHECK(controlflow_tokens[6].text == "loop");

                CHECK(controlflow_tokens[7].type == KW_while);
                CHECK(controlflow_tokens[7].text == "while");

                CHECK(controlflow_tokens[8].type == KW_for);
                CHECK(controlflow_tokens[8].text == "for");

                CHECK(controlflow_tokens[9].type == KW_break);
                CHECK(controlflow_tokens[9].text == "break");

                CHECK(controlflow_tokens[10].type == KW_skip);
                CHECK(controlflow_tokens[10].text == "skip");
            }
        }
    }

    SECTION("Operator lexing") {
        sigil::Lexer lexer;
        REQUIRE(lexer.Tokenize(Concatenate(LEXER_TESTING_PATH, "operators.mn")));

        const auto tokens = StripRedundant(lexer.RelinquishTokens());
        REQUIRE(tokens.size() == 79);

        SECTION("Logical Operators") {
            CHECK(tokens[12].type == Op_LogicalNot);
            CHECK(tokens[12].text == "!");

            CHECK(tokens[15].type == Op_Equality);
            CHECK(tokens[15].text == "==");

            CHECK(tokens[18].type == Op_NotEqual);
            CHECK(tokens[18].text == "!=");

            CHECK(tokens[21].type == Op_LessThan);
            CHECK(tokens[21].text == "<");

            CHECK(tokens[24].type == Op_GreaterThan);
            CHECK(tokens[24].text == ">");

            CHECK(tokens[27].type == Op_LessEqual);
            CHECK(tokens[27].text == "<=");

            CHECK(tokens[30].type == Op_GreaterEqual);
            CHECK(tokens[30].text == ">=");

            CHECK(tokens[33].type == Op_LogicalAnd);
            CHECK(tokens[33].text == "and");

            CHECK(tokens[36].type == Op_LogicalOr);
            CHECK(tokens[36].text == "or");

            CHECK(tokens[76].type == Op_LogicalNot);
            CHECK(tokens[76].text == "not");
        }

        SECTION("Arithmetic Operators") {
            CHECK(tokens[39].type == Op_Plus);
            CHECK(tokens[39].text == "+");

            CHECK(tokens[42].type == Op_Minus);
            CHECK(tokens[42].text == "-");

            CHECK(tokens[45].type == Op_Asterisk);
            CHECK(tokens[45].text == "*");

            CHECK(tokens[48].type == Op_FwdSlash);
            CHECK(tokens[48].text == "/");
        }

        SECTION("Access Operators") {
            CHECK(tokens[1].type == Op_Period);
            CHECK(tokens[1].text == ".");

            CHECK(tokens[7].type == Op_ModuleElementAccess);
            CHECK(tokens[7].text == "::");

            CHECK(tokens[51].type == Op_Arrow);
            CHECK(tokens[51].text == "->");

            CHECK(tokens[55].type == Op_BracketLeft);
            CHECK(tokens[55].text == "[");

            CHECK(tokens[56].type == Op_BracketRight);
            CHECK(tokens[56].text == "]");
        }

        SECTION("Assignment Operators") {
            CHECK(tokens[4].type == Op_Colon);
            CHECK(tokens[4].text == ":");

            CHECK(tokens[10].type == Op_Assign);
            CHECK(tokens[10].text == "=");

            CHECK(tokens[64].type == Op_ExplicitCopy);
            CHECK(tokens[64].text == "$");

            CHECK(tokens[68].type == Op_ExplicitMove);
            CHECK(tokens[68].text == "~");

            CHECK(tokens[72].type == Op_ExplicitRef);
            CHECK(tokens[72].text == "&");
        }

        SECTION("Structural Operators") {
            CHECK(tokens[53].type == Op_BraceLeft);
            CHECK(tokens[53].text == "{");

            CHECK(tokens[54].type == Op_BraceRight);
            CHECK(tokens[54].text == "}");

            CHECK(tokens[57].type == Op_ParenLeft);
            CHECK(tokens[57].text == "(");

            CHECK(tokens[58].type == Op_ParenRight);
            CHECK(tokens[58].text == ")");

            CHECK(tokens[60].type == Op_Comma);
            CHECK(tokens[60].text == ",");
        }

        SECTION("Textual Operators") {
            CHECK(tokens[74].type == Lit_String);
            CHECK(tokens[74].text == "\"hello world\"");

            CHECK(tokens[75].type == Lit_Char);
            CHECK(tokens[75].text == "\'f\'");
        }
    }
}