#include <catch2/catch_test_macros.hpp>

#include <salem/frontend/lexer.hpp>
#include "common.hpp"


using enum salem::token_type;
constexpr auto LEXER_TESTING_PATH = "Samples/Lexing/";
TEST_CASE("Lexer", "[lex][token][operator][keyword]") {
    SECTION("MFT", "Minimal functionality test") {
        salem::lexer lexer;
        REQUIRE(lexer.tokenize_file(concatenate(LEXER_TESTING_PATH, "Basic.mn")));

        const auto tokens = strip_redundant(lexer.relinquish_tokens());
        REQUIRE(tokens.size() == 16);

        SECTION("Tokens are being processed") {
            CHECK(tokens[0].type_ == KW_data);
            CHECK(tokens[0].text_ == "data");

            CHECK(tokens[1].type_ == Identifier);
            CHECK(tokens[1].text_ == "x");

            CHECK(tokens[2].type_ == Op_Assign);
            CHECK(tokens[2].text_ == "=");

            CHECK(tokens[3].type_ == Lit_Int);
            CHECK(tokens[3].text_ == "5");
        }

        SECTION("Tokens are correctly registered") {
            CHECK(tokens[4].type_ == Lit_Float);
            CHECK(tokens[4].text_ == "10.783");

            CHECK(tokens[5].type_ == KW_data);
            CHECK(tokens[5].text_ == "data");

            CHECK(tokens[6].type_ == Identifier);
            CHECK(tokens[6].text_ == "add");

            CHECK(tokens[7].type_ == Op_Assign);
            CHECK(tokens[7].text_ == "=");

            CHECK(tokens[8].type_ == Op_ParenLeft);
            CHECK(tokens[8].text_ == "(");

            CHECK(tokens[9].type_ == Op_ParenRight);
            CHECK(tokens[9].text_ == ")");

            CHECK(tokens[10].type_ == Op_BraceLeft);
            CHECK(tokens[10].text_ == "{");

            CHECK(tokens[11].type_ == Identifier);
            CHECK(tokens[11].text_ == "x");

            CHECK(tokens[12].type_ == Op_Plus);
            CHECK(tokens[12].text_ == "+");

            CHECK(tokens[13].type_ == Identifier);
            CHECK(tokens[13].text_ == "y");

            CHECK(tokens[14].type_ == Op_BraceRight);
            CHECK(tokens[14].text_ == "}");

            CHECK(tokens[15].type_ == Eof);
            CHECK(tokens[15].text_ == "EOF");
        }
    }

    const auto KEYWORD_TESTING_PATH = concatenate(LEXER_TESTING_PATH, "Keywords/");
    SECTION("Keyword lexing") {
        salem::lexer lexer;
        REQUIRE(lexer.tokenize_file(concatenate(KEYWORD_TESTING_PATH, "Datatypes.mn")));

        const auto datatype_tokens = strip_redundant(lexer.relinquish_tokens());
        REQUIRE(datatype_tokens.size() == 20);

        SECTION("Primitive Datatypes") {
            SECTION("Integers") {
                CHECK(datatype_tokens[0].type_ == KW_i8);
                CHECK(datatype_tokens[0].text_ == "i8");

                CHECK(datatype_tokens[1].type_ == KW_i16);
                CHECK(datatype_tokens[1].text_ == "i16");

                CHECK(datatype_tokens[2].type_ == KW_i32);
                CHECK(datatype_tokens[2].text_ == "i32");

                CHECK(datatype_tokens[3].type_ == KW_i64);
                CHECK(datatype_tokens[3].text_ == "i64");

                CHECK(datatype_tokens[4].type_ == KW_i128);
                CHECK(datatype_tokens[4].text_ == "i128");

                CHECK(datatype_tokens[5].type_ == KW_u8);
                CHECK(datatype_tokens[5].text_ == "u8");

                CHECK(datatype_tokens[6].type_ == KW_u16);
                CHECK(datatype_tokens[6].text_ == "u16");

                CHECK(datatype_tokens[7].type_ == KW_u32);
                CHECK(datatype_tokens[7].text_ == "u32");

                CHECK(datatype_tokens[8].type_ == KW_u64);
                CHECK(datatype_tokens[8].text_ == "u64");

                CHECK(datatype_tokens[9].type_ == KW_u128);
                CHECK(datatype_tokens[9].text_ == "u128");

                CHECK(datatype_tokens[12].type_ == KW_byte);
                CHECK(datatype_tokens[12].text_ == "byte");
            }

            SECTION("Floats") {
                CHECK(datatype_tokens[10].type_ == KW_f32);
                CHECK(datatype_tokens[10].text_ == "f32");

                CHECK(datatype_tokens[11].type_ == KW_f64);
                CHECK(datatype_tokens[11].text_ == "f64");
            }

            SECTION("Textual Types") {
                CHECK(datatype_tokens[13].type_ == KW_char);
                CHECK(datatype_tokens[13].text_ == "char");

                CHECK(datatype_tokens[14].type_ == KW_string);
                CHECK(datatype_tokens[14].text_ == "string");
            }

            SECTION("Misc") {
                CHECK(datatype_tokens[15].type_ == KW_bool);
                CHECK(datatype_tokens[15].text_ == "bool");

                CHECK(datatype_tokens[16].type_ == KW_void);
                CHECK(datatype_tokens[16].text_ == "void");

                CHECK(datatype_tokens[17].type_ == Op_ParenLeft);
                CHECK(datatype_tokens[17].text_ == "(");

                CHECK(datatype_tokens[18].type_ == Op_ParenRight);
                CHECK(datatype_tokens[18].text_ == ")");
            }
        }

        REQUIRE(lexer.tokenize_file(concatenate(KEYWORD_TESTING_PATH, "Declarations.mn")));

        const auto declaration_tokens = strip_redundant(lexer.relinquish_tokens());
        REQUIRE(declaration_tokens.size() == 16);

        SECTION("Declarations") {
            SECTION("Data Declarators") {
                CHECK(declaration_tokens[0].type_ == KW_data);
                CHECK(declaration_tokens[0].text_ == "data");

                CHECK(declaration_tokens[1].type_ == KW_fn);
                CHECK(declaration_tokens[1].text_ == "fn");

                CHECK(declaration_tokens[2].type_ == KW_mut);
                CHECK(declaration_tokens[2].text_ == "mut");

                CHECK(declaration_tokens[3].type_ == KW_const);
                CHECK(declaration_tokens[3].text_ == "const");

                CHECK(declaration_tokens[4].type_ == KW_raw);
                CHECK(declaration_tokens[4].text_ == "raw");

                CHECK(declaration_tokens[13].type_ == KW_override);
                CHECK(declaration_tokens[13].text_ == "override");
            }

            SECTION("UDT Declarators") {
                CHECK(declaration_tokens[5].type_ == KW_pack);
                CHECK(declaration_tokens[5].text_ == "pack");

                CHECK(declaration_tokens[6].type_ == KW_struct);
                CHECK(declaration_tokens[6].text_ == "struct");

                CHECK(declaration_tokens[7].type_ == KW_enum);
                CHECK(declaration_tokens[7].text_ == "enum");

                CHECK(declaration_tokens[12].type_ == KW_generic);
                CHECK(declaration_tokens[12].text_ == "generic");
            }

            SECTION("Module Declarators") {
                CHECK(declaration_tokens[8].type_ == KW_module);
                CHECK(declaration_tokens[8].text_ == "module");

                CHECK(declaration_tokens[9].type_ == KW_public);
                CHECK(declaration_tokens[9].text_ == "public");

                CHECK(declaration_tokens[10].type_ == KW_private);
                CHECK(declaration_tokens[10].text_ == "private");

                CHECK(declaration_tokens[11].type_ == KW_import);
                CHECK(declaration_tokens[11].text_ == "import");

                CHECK(declaration_tokens[14].type_ == KW_as);
                CHECK(declaration_tokens[14].text_ == "as");
            }
        }

        REQUIRE(lexer.tokenize_file(concatenate(KEYWORD_TESTING_PATH, "ControlFlow.mn")));

        const auto controlflow_tokens = strip_redundant(lexer.relinquish_tokens());
        REQUIRE(controlflow_tokens.size() == 12);

        SECTION("Control Flow") {
            SECTION("Branching") {
                CHECK(controlflow_tokens[0].type_ == KW_return);
                CHECK(controlflow_tokens[0].text_ == "return");

                CHECK(controlflow_tokens[1].type_ == KW_true);
                CHECK(controlflow_tokens[1].text_ == "true");

                CHECK(controlflow_tokens[2].type_ == KW_false);
                CHECK(controlflow_tokens[2].text_ == "false");

                CHECK(controlflow_tokens[3].type_ == KW_if);
                CHECK(controlflow_tokens[3].text_ == "if");

                CHECK(controlflow_tokens[4].type_ == KW_else);
                CHECK(controlflow_tokens[4].text_ == "else");

                CHECK(controlflow_tokens[5].type_ == KW_match);
                CHECK(controlflow_tokens[5].text_ == "match");
            }

            SECTION("Iteration") {

                CHECK(controlflow_tokens[6].type_ == KW_loop);
                CHECK(controlflow_tokens[6].text_ == "loop");

                CHECK(controlflow_tokens[7].type_ == KW_while);
                CHECK(controlflow_tokens[7].text_ == "while");

                CHECK(controlflow_tokens[8].type_ == KW_for);
                CHECK(controlflow_tokens[8].text_ == "for");

                CHECK(controlflow_tokens[9].type_ == KW_break);
                CHECK(controlflow_tokens[9].text_ == "break");

                CHECK(controlflow_tokens[10].type_ == KW_skip);
                CHECK(controlflow_tokens[10].text_ == "skip");
            }
        }

    }

    SECTION("Operator lexing") {
        salem::lexer lexer;
        REQUIRE(lexer.tokenize_file(concatenate(LEXER_TESTING_PATH, "Operators.mn")));

        const auto tokens = strip_redundant(lexer.relinquish_tokens());
        REQUIRE(tokens.size() == 79);

        SECTION("Logical Operators") {
            CHECK(tokens[12].type_ == Op_LogicalNot);
            CHECK(tokens[12].text_ == "!");

            CHECK(tokens[15].type_ == Op_Equality);
            CHECK(tokens[15].text_ == "==");

            CHECK(tokens[18].type_ == Op_NotEqual);
            CHECK(tokens[18].text_ == "!=");

            CHECK(tokens[21].type_ == Op_LessThan);
            CHECK(tokens[21].text_ == "<");

            CHECK(tokens[24].type_ == Op_GreaterThan);
            CHECK(tokens[24].text_ == ">");

            CHECK(tokens[27].type_ == Op_LessEqual);
            CHECK(tokens[27].text_ == "<=");

            CHECK(tokens[30].type_ == Op_GreaterEqual);
            CHECK(tokens[30].text_ == ">=");

            CHECK(tokens[33].type_ == Op_LogicalAnd);
            CHECK(tokens[33].text_ == "and");

            CHECK(tokens[36].type_ == Op_LogicalOr);
            CHECK(tokens[36].text_ == "or");

            CHECK(tokens[76].type_ == Op_LogicalNot);
            CHECK(tokens[76].text_ == "not");

        }

        SECTION("Arithmetic Operators") {
            CHECK(tokens[39].type_ == Op_Plus);
            CHECK(tokens[39].text_ == "+");

            CHECK(tokens[42].type_ == Op_Minus);
            CHECK(tokens[42].text_ == "-");

            CHECK(tokens[45].type_ == Op_Asterisk);
            CHECK(tokens[45].text_ == "*");

            CHECK(tokens[48].type_ == Op_FwdSlash);
            CHECK(tokens[48].text_ == "/");
        }

        SECTION("Access Operators") {
            CHECK(tokens[1].type_ == Op_Period);
            CHECK(tokens[1].text_ == ".");

            CHECK(tokens[7].type_ == Op_ModuleElementAccess);
            CHECK(tokens[7].text_ == "::");

            CHECK(tokens[51].type_ == Op_Arrow);
            CHECK(tokens[51].text_ == "->");

            CHECK(tokens[55].type_ == Op_BracketLeft);
            CHECK(tokens[55].text_ == "[");

            CHECK(tokens[56].type_ == Op_BracketRight);
            CHECK(tokens[56].text_ == "]");
        }

        SECTION("Assignment Operators") {
            CHECK(tokens[4].type_ == Op_Colon);
            CHECK(tokens[4].text_ == ":");

            CHECK(tokens[10].type_ == Op_Assign);
            CHECK(tokens[10].text_ == "=");

            CHECK(tokens[64].type_ == Op_ExplicitCopy);
            CHECK(tokens[64].text_ == "$");

            CHECK(tokens[68].type_ == Op_ExplicitMove);
            CHECK(tokens[68].text_ == "~");

            CHECK(tokens[72].type_ == Op_ExplicitRef);
            CHECK(tokens[72].text_ == "&");
        }

        SECTION("Structural Operators") {
            CHECK(tokens[53].type_ == Op_BraceLeft);
            CHECK(tokens[53].text_ == "{");

            CHECK(tokens[54].type_ == Op_BraceRight);
            CHECK(tokens[54].text_ == "}");

            CHECK(tokens[57].type_ == Op_ParenLeft);
            CHECK(tokens[57].text_ == "(");

            CHECK(tokens[58].type_ == Op_ParenRight);
            CHECK(tokens[58].text_ == ")");

            CHECK(tokens[60].type_ == Op_Comma);
            CHECK(tokens[60].text_ == ",");
        }

        SECTION("Textual Operators") {
            CHECK(tokens[74].type_ == Lit_String);
            CHECK(tokens[74].text_ == "\"hello world\"");

            CHECK(tokens[75].type_ == Lit_Char);
            CHECK(tokens[75].text_ == "\'f\'");
        }
    }
}