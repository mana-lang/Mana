#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <Salem/FrontEnd/Lexer.hpp>
#include <Salem/Core/TypeAliases.hpp>

template<class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template<StringLike String, StringLike... Rest>
auto Concatenate(String first, Rest... rest) {
    return std::string(first) + std::string(rest...);
}

using TokenStream = std::vector<salem::Token>;
auto StripNewlinesFromTokens(const TokenStream& base_tokens) -> TokenStream {
    auto tokens = decltype(base_tokens){};
    tokens.reserve(base_tokens.size());
    for (const auto& t : base_tokens) {
        if (t.type == salem::Token::Type::Newline) {
            continue;
        }
        tokens.push_back(t);
    }

    return tokens;
}

constexpr auto LEXER_TESTING_PATH = "TestingSamples/Lexing/";
TEST_CASE("Basic Lexing", "[lex][token]") {
    salem::Lexer lexer;
    REQUIRE(lexer.TokenizeFile(Concatenate(LEXER_TESTING_PATH, "Basic.mn")));

    const auto tokens = StripNewlinesFromTokens(lexer.RelinquishTokens());
    REQUIRE(tokens.size() == 16);

    SECTION("Tokens are being processed") {
        CHECK(tokens[0].type == salem::Token::Type::KW_data);
        CHECK(tokens[0].contents == "data");

        CHECK(tokens[1].type == salem::Token::Type::Identifier);
        CHECK(tokens[1].contents == "x");

        CHECK(tokens[2].type == salem::Token::Type::Op_Assign);
        CHECK(tokens[2].contents == "=");

        CHECK(tokens[3].type == salem::Token::Type::Int);
        CHECK(tokens[3].contents == "5");
    }

    SECTION("Tokens are correctly registered") {
        CHECK(tokens[4].type == salem::Token::Type::Float);
        CHECK(tokens[4].contents == "10.783");

        CHECK(tokens[5].type == salem::Token::Type::KW_data);
        CHECK(tokens[5].contents == "data");

        CHECK(tokens[6].type == salem::Token::Type::Identifier);
        CHECK(tokens[6].contents == "add");

        CHECK(tokens[7].type == salem::Token::Type::Op_Assign);
        CHECK(tokens[7].contents == "=");

        CHECK(tokens[8].type == salem::Token::Type::Op_ParenLeft);
        CHECK(tokens[8].contents == "(");

        CHECK(tokens[9].type == salem::Token::Type::Op_ParenRight);
        CHECK(tokens[9].contents == ")");

        CHECK(tokens[10].type == salem::Token::Type::Op_BraceLeft);
        CHECK(tokens[10].contents == "{");

        CHECK(tokens[11].type == salem::Token::Type::Identifier);
        CHECK(tokens[11].contents == "x");

        CHECK(tokens[12].type == salem::Token::Type::Op_Plus);
        CHECK(tokens[12].contents == "+");

        CHECK(tokens[13].type == salem::Token::Type::Identifier);
        CHECK(tokens[13].contents == "y");

        CHECK(tokens[14].type == salem::Token::Type::Op_BraceRight);
        CHECK(tokens[14].contents == "}");

        CHECK(tokens[15].type == salem::Token::Type::Eof);
        CHECK(tokens[15].contents == "EOF");
    }
}

const auto KEYWORD_TESTING_PATH = Concatenate(LEXER_TESTING_PATH, "Keywords/");
TEST_CASE("Lexing Keywords", "[lex][token][keyword]") {
    salem::Lexer lexer;
    REQUIRE(lexer.TokenizeFile(Concatenate(KEYWORD_TESTING_PATH, "Datatypes.mn")));

    const auto datatype_tokens = StripNewlinesFromTokens(lexer.RelinquishTokens());
    REQUIRE(datatype_tokens.size() == 20);

    SECTION("Primitive Datatypes") {
        SECTION("Integers") {
            CHECK(datatype_tokens[0].type == salem::Token::Type::KW_i8);
            CHECK(datatype_tokens[0].contents == "i8");

            CHECK(datatype_tokens[1].type == salem::Token::Type::KW_i16);
            CHECK(datatype_tokens[1].contents == "i16");

            CHECK(datatype_tokens[2].type == salem::Token::Type::KW_i32);
            CHECK(datatype_tokens[2].contents == "i32");

            CHECK(datatype_tokens[3].type == salem::Token::Type::KW_i64);
            CHECK(datatype_tokens[3].contents == "i64");

            CHECK(datatype_tokens[4].type == salem::Token::Type::KW_i128);
            CHECK(datatype_tokens[4].contents == "i128");

            CHECK(datatype_tokens[5].type == salem::Token::Type::KW_u8);
            CHECK(datatype_tokens[5].contents == "u8");

            CHECK(datatype_tokens[6].type == salem::Token::Type::KW_u16);
            CHECK(datatype_tokens[6].contents == "u16");

            CHECK(datatype_tokens[7].type == salem::Token::Type::KW_u32);
            CHECK(datatype_tokens[7].contents == "u32");

            CHECK(datatype_tokens[8].type == salem::Token::Type::KW_u64);
            CHECK(datatype_tokens[8].contents == "u64");

            CHECK(datatype_tokens[9].type == salem::Token::Type::KW_u128);
            CHECK(datatype_tokens[9].contents == "u128");

            CHECK(datatype_tokens[12].type == salem::Token::Type::KW_byte);
            CHECK(datatype_tokens[12].contents == "byte");
        }

        SECTION("Floats") {
            CHECK(datatype_tokens[10].type == salem::Token::Type::KW_f32);
            CHECK(datatype_tokens[10].contents == "f32");

            CHECK(datatype_tokens[11].type == salem::Token::Type::KW_f64);
            CHECK(datatype_tokens[11].contents == "f64");
        }

        SECTION("Textual Types") {
            CHECK(datatype_tokens[13].type == salem::Token::Type::KW_char);
            CHECK(datatype_tokens[13].contents == "char");

            CHECK(datatype_tokens[14].type == salem::Token::Type::KW_string);
            CHECK(datatype_tokens[14].contents == "string");
        }

        SECTION("Misc") {
            CHECK(datatype_tokens[15].type == salem::Token::Type::KW_bool);
            CHECK(datatype_tokens[15].contents == "bool");

            CHECK(datatype_tokens[16].type == salem::Token::Type::KW_void);
            CHECK(datatype_tokens[16].contents == "void");

            CHECK(datatype_tokens[17].type == salem::Token::Type::Op_ParenLeft);
            CHECK(datatype_tokens[17].contents == "(");

            CHECK(datatype_tokens[18].type == salem::Token::Type::Op_ParenRight);
            CHECK(datatype_tokens[18].contents == ")");
        }
    }

    REQUIRE(lexer.TokenizeFile(Concatenate(KEYWORD_TESTING_PATH, "Declarations.mn")));


    const auto declaration_tokens = StripNewlinesFromTokens(lexer.RelinquishTokens());
    REQUIRE(declaration_tokens.size() == 16);

    SECTION("Declarations") {
        SECTION("Data Declarators") {
            CHECK(declaration_tokens[0].type == salem::Token::Type::KW_data);
            CHECK(declaration_tokens[0].contents == "data");

            CHECK(declaration_tokens[1].type == salem::Token::Type::KW_fn);
            CHECK(declaration_tokens[1].contents == "fn");

            CHECK(declaration_tokens[2].type == salem::Token::Type::KW_mut);
            CHECK(declaration_tokens[2].contents == "mut");

            CHECK(declaration_tokens[3].type == salem::Token::Type::KW_const);
            CHECK(declaration_tokens[3].contents == "const");

            CHECK(declaration_tokens[4].type == salem::Token::Type::KW_raw);
            CHECK(declaration_tokens[4].contents == "raw");

            CHECK(declaration_tokens[13].type == salem::Token::Type::KW_override);
            CHECK(declaration_tokens[13].contents == "override");
        }

        SECTION("UDT Declarators") {
            CHECK(declaration_tokens[5].type == salem::Token::Type::KW_pack);
            CHECK(declaration_tokens[5].contents == "pack");

            CHECK(declaration_tokens[6].type == salem::Token::Type::KW_struct);
            CHECK(declaration_tokens[6].contents == "struct");

            CHECK(declaration_tokens[7].type == salem::Token::Type::KW_enum);
            CHECK(declaration_tokens[7].contents == "enum");

            CHECK(declaration_tokens[12].type == salem::Token::Type::KW_generic);
            CHECK(declaration_tokens[12].contents == "generic");
        }

        SECTION("Module Declarators") {
            CHECK(declaration_tokens[8].type == salem::Token::Type::KW_module);
            CHECK(declaration_tokens[8].contents == "module");

            CHECK(declaration_tokens[9].type == salem::Token::Type::KW_public);
            CHECK(declaration_tokens[9].contents == "public");

            CHECK(declaration_tokens[10].type == salem::Token::Type::KW_private);
            CHECK(declaration_tokens[10].contents == "private");

            CHECK(declaration_tokens[11].type == salem::Token::Type::KW_import);
            CHECK(declaration_tokens[11].contents == "import");

            CHECK(declaration_tokens[14].type == salem::Token::Type::KW_as);
            CHECK(declaration_tokens[14].contents == "as");
        }
    }

    REQUIRE(lexer.TokenizeFile(Concatenate(KEYWORD_TESTING_PATH, "ControlFlow.mn")));


    const auto controlflow_tokens = StripNewlinesFromTokens(lexer.RelinquishTokens());
    REQUIRE(controlflow_tokens.size() == 12);

    SECTION("Control Flow") {
        SECTION("Branching") {
            CHECK(controlflow_tokens[0].type == salem::Token::Type::KW_return);
            CHECK(controlflow_tokens[0].contents == "return");

            CHECK(controlflow_tokens[1].type == salem::Token::Type::KW_true);
            CHECK(controlflow_tokens[1].contents == "true");

            CHECK(controlflow_tokens[2].type == salem::Token::Type::KW_false);
            CHECK(controlflow_tokens[2].contents == "false");

            CHECK(controlflow_tokens[3].type == salem::Token::Type::KW_if);
            CHECK(controlflow_tokens[3].contents == "if");

            CHECK(controlflow_tokens[4].type == salem::Token::Type::KW_else);
            CHECK(controlflow_tokens[4].contents == "else");

            CHECK(controlflow_tokens[5].type == salem::Token::Type::KW_match);
            CHECK(controlflow_tokens[5].contents == "match");
        }

        SECTION("Iteration") {

            CHECK(controlflow_tokens[6].type == salem::Token::Type::KW_loop);
            CHECK(controlflow_tokens[6].contents == "loop");

            CHECK(controlflow_tokens[7].type == salem::Token::Type::KW_while);
            CHECK(controlflow_tokens[7].contents == "while");

            CHECK(controlflow_tokens[8].type == salem::Token::Type::KW_for);
            CHECK(controlflow_tokens[8].contents == "for");

            CHECK(controlflow_tokens[9].type == salem::Token::Type::KW_break);
            CHECK(controlflow_tokens[9].contents == "break");

            CHECK(controlflow_tokens[10].type == salem::Token::Type::KW_skip);
            CHECK(controlflow_tokens[10].contents == "skip");
        }
    }

}

TEST_CASE("Lexing Operators", "[lex][token][operator]") {
    salem::Lexer lexer;
    REQUIRE(lexer.TokenizeFile(Concatenate(LEXER_TESTING_PATH, "Operators.mn")));

    const auto tokens = StripNewlinesFromTokens(lexer.RelinquishTokens());
    REQUIRE(tokens.size() == 84);

    CHECK(tokens[1].type == salem::Token::Type::Op_Period);
    CHECK(tokens[1].contents == ".");

    CHECK(tokens[1].type == salem::Token::Type::Op_Colon);
    CHECK(tokens[1].contents == ":");

    CHECK(tokens[2].type == salem::Token::Type::Op_ModuleElementAccess);
    CHECK(tokens[2].contents == "::");

    CHECK(tokens[3].type == salem::Token::Type::Op_Assign);
    CHECK(tokens[3].contents == "=");

    CHECK(tokens[4].type == salem::Token::Type::Op_LogicalNot);
    CHECK(tokens[4].contents == "!");

    CHECK(tokens[5].type == salem::Token::Type::Op_Equality);
    CHECK(tokens[5].contents == "==");

    CHECK(tokens[6].type == salem::Token::Type::Op_NotEqual);
    CHECK(tokens[6].contents == "!=");

    CHECK(tokens[7].type == salem::Token::Type::Op_LessThan);
    CHECK(tokens[7].contents == "<");

    CHECK(tokens[8].type == salem::Token::Type::Op_GreaterThan);
    CHECK(tokens[8].contents == ">");

    CHECK(tokens[9].type == salem::Token::Type::Op_LessEqual);
    CHECK(tokens[9].contents == "<=");

    CHECK(tokens[12].type == salem::Token::Type::Op_GreaterEqual);
    CHECK(tokens[12].contents == ">=");
    
}