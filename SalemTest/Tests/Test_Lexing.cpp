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

constexpr auto LEXER_TESTING_PATH = "ManaExamples/Lexing/";
TEST_CASE("Basic Lexing", "[lex][token]") {
    salem::Lexer lexer;
    REQUIRE(lexer.TokenizeFile(Concatenate(LEXER_TESTING_PATH, "Basic.mn")));

    const auto base_tokens = lexer.RelinquishTokens();
    REQUIRE_FALSE(base_tokens.empty());

    const auto tokens = StripNewlinesFromTokens(base_tokens);
    REQUIRE(tokens.size() == 16);

    SECTION("Tokens are being processed") {
        CHECK(tokens[0].type == salem::Token::Type::Identifier);
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

        CHECK(tokens[5].type == salem::Token::Type::Identifier);
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

        CHECK(tokens[12].type == salem::Token::Type::Op_Add);
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

    const auto base_tokens = lexer.RelinquishTokens();
    REQUIRE_FALSE(base_tokens.empty());

    const auto tokens = StripNewlinesFromTokens(base_tokens);
    REQUIRE(tokens.size() == 20);

    SECTION("Datatypes") {
        CHECK(tokens[0].type == salem::Token::Type::KW_i8);
        CHECK(tokens[0].contents == "i8");

        CHECK(tokens[1].type == salem::Token::Type::KW_i16);
        CHECK(tokens[1].contents == "i16");

        CHECK(tokens[2].type == salem::Token::Type::KW_i32);
        CHECK(tokens[2].contents == "i32");

        CHECK(tokens[3].type == salem::Token::Type::KW_i64);
        CHECK(tokens[3].contents == "i64");

        CHECK(tokens[4].type == salem::Token::Type::KW_i128);
        CHECK(tokens[4].contents == "i128");

        CHECK(tokens[5].type == salem::Token::Type::KW_u8);
        CHECK(tokens[5].contents == "u8");

        CHECK(tokens[6].type == salem::Token::Type::KW_u16);
        CHECK(tokens[6].contents == "u16");

        CHECK(tokens[7].type == salem::Token::Type::KW_u32);
        CHECK(tokens[7].contents == "u32");

        CHECK(tokens[8].type == salem::Token::Type::KW_u64);
        CHECK(tokens[8].contents == "u64");

        CHECK(tokens[9].type == salem::Token::Type::KW_u128);
        CHECK(tokens[9].contents == "u128");

        CHECK(tokens[10].type == salem::Token::Type::KW_f32);
        CHECK(tokens[10].contents == "f32");

        CHECK(tokens[11].type == salem::Token::Type::KW_f64);
        CHECK(tokens[11].contents == "f64");

        CHECK(tokens[12].type == salem::Token::Type::KW_byte);
        CHECK(tokens[12].contents == "byte");

        CHECK(tokens[13].type == salem::Token::Type::KW_char);
        CHECK(tokens[13].contents == "char");

        CHECK(tokens[14].type == salem::Token::Type::KW_string);
        CHECK(tokens[14].contents == "string");

        CHECK(tokens[15].type == salem::Token::Type::KW_bool);
        CHECK(tokens[15].contents == "bool");

        CHECK(tokens[16].type == salem::Token::Type::KW_void);
        CHECK(tokens[16].contents == "void");

        CHECK(tokens[17].type == salem::Token::Type::Op_ParenLeft);
        CHECK(tokens[17].contents == "(");

        CHECK(tokens[18].type == salem::Token::Type::Op_ParenRight);
        CHECK(tokens[18].contents == ")");

        CHECK(tokens[19].type == salem::Token::Type::Eof);
        CHECK(tokens[19].contents == "EOF");
    }
}