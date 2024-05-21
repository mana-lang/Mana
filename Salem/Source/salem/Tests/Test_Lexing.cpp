#include <catch2/catch_test_macros.hpp>

#include <Salem/FrontEnd/Lexer.hpp>
#include <Salem/Core/TypeAliases.hpp>

constexpr salem::CString BASIC_LEXING_PATH = "ManaExamples/BasicLexing.mn";
TEST_CASE("Basic Lexing", "[lex][token]") {
    salem::Lexer lexer;
    REQUIRE(lexer.TokenizeFile(BASIC_LEXING_PATH));

    const auto base_tokens = lexer.RelinquishTokens();
    REQUIRE_FALSE(base_tokens.empty());

    auto tokens = decltype(base_tokens){};
    tokens.reserve(base_tokens.size());
    for (const auto& t : base_tokens) {
        if (t.type == salem::Token::Type::Newline) {
            continue;
        }
        tokens.push_back(t);
    }

    SECTION("Tokens are properly processed") {
        CHECK(tokens[0].type == salem::Token::Type::Identifier);
        CHECK(tokens[0].contents == "data");

        CHECK(tokens[1].type == salem::Token::Type::Identifier);
        CHECK(tokens[1].contents == "x");

        CHECK(tokens[2].type == salem::Token::Type::OpAssign);
        CHECK(tokens[2].contents == "=");

        CHECK(tokens[3].type == salem::Token::Type::Int);
        CHECK(tokens[3].contents == "5");


        CHECK(tokens[4].type == salem::Token::Type::Identifier);
        CHECK(tokens[4].contents == "data");

        CHECK(tokens[5].type == salem::Token::Type::Identifier);
        CHECK(tokens[5].contents == "ten");

        CHECK(tokens[6].type == salem::Token::Type::OpAssign);
        CHECK(tokens[6].contents == "=");

        CHECK(tokens[7].type == salem::Token::Type::Float);
        CHECK(tokens[7].contents == "10.783");


        CHECK(tokens[8].type == salem::Token::Type::Identifier);
        CHECK(tokens[8].contents == "data");

        CHECK(tokens[9].type == salem::Token::Type::Identifier);
        CHECK(tokens[9].contents == "add");

        CHECK(tokens[10].type == salem::Token::Type::OpAssign);
        CHECK(tokens[10].contents == "=");

        CHECK(tokens[11].type == salem::Token::Type::OpParenLeft);
        CHECK(tokens[11].contents == "(");

        CHECK(tokens[12].type == salem::Token::Type::OpParenRight);
        CHECK(tokens[12].contents == ")");

        CHECK(tokens[13].type == salem::Token::Type::OpBraceLeft);
        CHECK(tokens[13].contents == "{");

        CHECK(tokens[14].type == salem::Token::Type::Identifier);
        CHECK(tokens[14].contents == "x");

        CHECK(tokens[15].type == salem::Token::Type::OpAdd);
        CHECK(tokens[15].contents == "+");

        CHECK(tokens[16].type == salem::Token::Type::Identifier);
        CHECK(tokens[16].contents == "y");

        CHECK(tokens[17].type == salem::Token::Type::OpBraceRight);
        CHECK(tokens[17].contents == "}");



        CHECK(tokens[18].type == salem::Token::Type::Identifier);
        CHECK(tokens[18].contents == "data");

        CHECK(tokens[19].type == salem::Token::Type::Identifier);
        CHECK(tokens[19].contents == "ten");

        CHECK(tokens[20].type == salem::Token::Type::OpAssign);
        CHECK(tokens[20].contents == "=");

        CHECK(tokens[21].type == salem::Token::Type::Identifier);
        CHECK(tokens[21].contents == "add");

        CHECK(tokens[22].type == salem::Token::Type::OpParenLeft);
        CHECK(tokens[22].contents == "(");

        CHECK(tokens[23].type == salem::Token::Type::Identifier);
        CHECK(tokens[23].contents == "five");

        CHECK(tokens[24].type == salem::Token::Type::OpComma);
        CHECK(tokens[24].contents == ",");

        CHECK(tokens[25].type == salem::Token::Type::Identifier);
        CHECK(tokens[25].contents == "ten");

        CHECK(tokens[26].type == salem::Token::Type::OpParenRight);
        CHECK(tokens[26].contents == ")");
    }
}