#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <hexec/value.hpp>

#include <vector>
#include <span>

using namespace hexec;
using Catch::Approx;

TEST_CASE("Hexe VM Generic Value type", "[value]") {
    SECTION("Construction") {
        SECTION("Default construction", "[value]") {
            Value v;
        }

        SECTION("From i64", "[value]") {
            Value v(static_cast<i64>(42));
            REQUIRE(v.Type() == Value::Data::Type::Int64);
            REQUIRE(v.AsInt() == 42);
        }


        SECTION("From i32", "[value]") {
            Value v(static_cast<i32>(-7));
            REQUIRE(v.Type() == Value::Data::Type::Int64);
            REQUIRE(v.AsInt() == -7);
        }

        SECTION("From u64", "[value]") {
            Value v(static_cast<u64>(100));
            REQUIRE(v.Type() == Value::Data::Type::Uint64);
            REQUIRE(v.AsUint() == 100);
        }

        SECTION("From u32", "[value]") {
            Value v(static_cast<u32>(50));
            REQUIRE(v.Type() == Value::Data::Type::Uint64);
            REQUIRE(v.AsUint() == 50);
        }

        SECTION("From f64", "[value]") {
            Value v(3.14);
            REQUIRE(v.Type() == Value::Data::Type::Float64);
            REQUIRE(v.AsFloat() == Approx(3.14));
        }

        SECTION("From bool", "[value]") {
            Value t(true), f(false);
            REQUIRE(t.Type() == Value::Data::Type::Bool);
            REQUIRE(t.AsBool() == true);
            REQUIRE(f.AsBool() == false);
        }

        SECTION("From string", "[value]") {
            Value v(std::string_view("hello"));
            REQUIRE(v.Type() == Value::Data::Type::String);
            REQUIRE(v.AsString() == "hello");

            Value v2("world");
            REQUIRE(v2.Type() == Value::Data::Type::String);
            REQUIRE(v2.AsString() == "world");
        }

        SECTION("From empty string", "[value]") {
            Value v(std::string_view(""));
            REQUIRE(v.Type() == Value::Data::Type::String);
            REQUIRE(v.AsString() == "");

            Value v2("");
            REQUIRE(v2.Type() == Value::Data::Type::String);
            REQUIRE(v2.AsString() == "");
        }
    }

    SECTION("Arithmetic (integers)") {
        SECTION("Integer addition", "[value][arithmetic]") {
            Value a(static_cast<i64>(10)), b(static_cast<i64>(3));
            REQUIRE((a + b).AsInt() == 13);
        }

        SECTION("Integer subtraction", "[value][arithmetic]") {
            Value a(static_cast<i64>(10)), b(static_cast<i64>(3));
            REQUIRE((a - b).AsInt() == 7);
        }

        SECTION("Integer multiplication", "[value][arithmetic]") {
            Value a(static_cast<i64>(10)), b(static_cast<i64>(3));
            REQUIRE((a * b).AsInt() == 30);
        }

        SECTION("Integer division", "[value][arithmetic]") {
            Value a(static_cast<i64>(10)), b(static_cast<i64>(3));
            REQUIRE((a / b).AsInt() == 3);
        }

        SECTION("Integer modulo", "[value][arithmetic]") {
            Value a(static_cast<i64>(10)), b(static_cast<i64>(3));
            REQUIRE((a % b).AsInt() == 1);
        }

        SECTION("Integer negation", "[value][arithmetic]") {
            Value a(static_cast<i64>(5));
            REQUIRE((-a).AsInt() == -5);
        }

        SECTION("Negative integer negation", "[value][arithmetic]") {
            Value a(static_cast<i64>(-42));
            REQUIRE((-a).AsInt() == 42);
        }
    }
}

