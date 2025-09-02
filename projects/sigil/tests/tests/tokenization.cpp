#include <catch2/catch_test_macros.hpp>

#include "headers/common.hpp"
#include <sigil/ast/lexer.hpp>

#include <filesystem>

constexpr auto LEXER_TESTING_PATH = "assets/samples/lexing/";
using namespace sigil;
namespace fs = std::filesystem;



TEST_CASE("Lexer", "[lex][token][operator][keyword]") {
    using enum TokenType;




}