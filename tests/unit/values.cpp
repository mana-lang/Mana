#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <hexec/value.hpp>

#include <vector>
#include <span>

using namespace hexec;
using Catch::Approx;

// ReSharper disable once CppDFATimeOver
TEST_CASE("VM Generic Value Type", "[value]") {
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

    SECTION("Arithmetic") {
        SECTION("Integers") {
            SECTION("Addition", "[value][arithmetic]") {
                Value a(10), b(3);
                REQUIRE((a + b).AsInt() == 13);
            }

            SECTION("Subtraction", "[value][arithmetic]") {
                Value a(10), b(3);
                REQUIRE((a - b).AsInt() == 7);
            }

            SECTION("Multiplication", "[value][arithmetic]") {
                Value a(10), b(3);
                REQUIRE((a * b).AsInt() == 30);
            }

            SECTION("Division", "[value][arithmetic]") {
                Value a(10), b(3);
                REQUIRE((a / b).AsInt() == 3);
            }

            SECTION("Modulo", "[value][arithmetic]") {
                Value a(10), b(3);
                REQUIRE((a % b).AsInt() == 1);
            }

            SECTION("Negation", "[value][arithmetic]") {
                Value a(5);
                REQUIRE((-a).AsInt() == -5);
            }

            SECTION("Double Negation", "[value][arithmetic]") {
                Value a(-42);
                REQUIRE((-a).AsInt() == 42);
            }

            SECTION("Unsigned Underflow", "[value][arithmetic]") {
                Value a(-1ul);
                REQUIRE(a.AsUint() == -1ul);
            }
        }

        SECTION("Floats") {
            SECTION("Addition", "[value][arithmetic]") {
                Value a(2.5), b(4.0);
                REQUIRE((a + b).AsFloat() == Approx(6.5));
            }

            SECTION("Subtraction", "[value][arithmetic]") {
                Value a(2.5), b(4.0);
                REQUIRE((a - b).AsFloat() == Approx(-1.5));
            }

            SECTION("Multiplication", "[value][arithmetic]") {
                Value a(2.5), b(4.0);
                REQUIRE((a * b).AsFloat() == Approx(10.0));
            }

            SECTION("Division", "[value][arithmetic]") {
                Value a(2.5), b(4.0);
                REQUIRE((a / b).AsFloat() == Approx(0.625));
            }

            SECTION("Modulo", "[value][arithmetic]") {
                Value a(7.53), b(2.38);
                REQUIRE((a % b).AsFloat() == Approx(0.39));
            }

            SECTION("Negation", "[value][arithmetic]") {
                Value a(2.5);
                REQUIRE((-a).AsFloat() == Approx(-2.5));
            }

            SECTION("Double Negation", "[value][arithmetic]") {
                Value a(-2.5);
                REQUIRE((-a).AsFloat() == Approx(2.5));
            }
        }
    }
    SECTION("Compound Assignment") {
        SECTION("Add", "[value][compound]") {
            Value a(5);
            a += Value(3);
            REQUIRE(a.AsInt() == 8);
        }

        SECTION("Subtract", "[value][compound]") {
            Value a(10);
            a -= Value(4);
            REQUIRE(a.AsInt() == 6);
        }

        SECTION("Multiply", "[value][compound]") {
            Value a(3);
            a *= Value(7);
            REQUIRE(a.AsInt() == 21);
        }

        SECTION("Divide", "[value][compound]") {
            Value a(20);
            a /= Value(4);
            REQUIRE(a.AsInt() == 5);
        }

        SECTION("Modulo", "[value][compound]") {
            Value a(10);
            a %= Value(3);
            REQUIRE(a.AsInt() == 1);
        }
    }

    SECTION("Comparison") {
        SECTION("Equality", "[value][comparison]") {
            Value a(10), b(10), c(20);
            REQUIRE(a == b);
            REQUIRE_FALSE(a == c);
        }

        SECTION("Less Than", "[value][comparison]") {
            Value a(10), b(20);
            REQUIRE(a < b);
            REQUIRE_FALSE(b < a);
        }

        SECTION("Greater Than", "[value][comparison]") {
            Value a(10), b(20);
            REQUIRE(b > a);
            REQUIRE_FALSE(a > b);
        }

        SECTION("Less Than or Equal", "[value][comparison]") {
            Value a(10), b(10), c(20);
            REQUIRE(a <= b);
            REQUIRE(a <= c);
            REQUIRE_FALSE(c <= a);
        }

        SECTION("Greater Than or Equal", "[value][comparison]") {
            Value a(10), b(10), c(20);
            REQUIRE(a >= b);
            REQUIRE(c >= a);
            REQUIRE_FALSE(a >= c);
        }

        SECTION("Logical not", "[value][comparison]") {
            Value t(true), f(false);
            REQUIRE(!f);
            REQUIRE_FALSE(!t);
        }
    }

    SECTION("Type Dispatch") {
        SECTION("From int to float", "[value][dispatch]") {
            Value v(static_cast<i64>(42));
            REQUIRE(v.AsFloat() == Approx(42.0));
        }

        SECTION("From float to int (truncation)", "[value][dispatch]") {
            Value v(3.7);
            REQUIRE(v.AsInt() == 3);
        }

        SECTION("From bool to int", "[value][dispatch]") {
            REQUIRE(Value(true).AsInt() == 1);
            REQUIRE(Value(false).AsInt() == 0);
        }

        SECTION("From bool to float", "[value][dispatch]") {
            REQUIRE(Value(true).AsFloat() == Approx(1.0));
            REQUIRE(Value(false).AsFloat() == Approx(0.0));
        }
    }

    SECTION("Lifecycle") {
        SECTION("Copy constructor", "[value][lifecycle]") {
            Value a(42);
            Value b(a);
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Copy assignment", "[value][lifecycle]") {
            Value a(42);
            Value b(0);
            b = a;
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Move constructor", "[value][lifecycle]") {
            Value a(42);
            Value b(std::move(a));
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move assignment", "[value][lifecycle]") {
            Value a(99);
            Value b(0);
            b = std::move(a);
            REQUIRE(b.AsInt() == 99);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }
    }

    SECTION("Arrays") {
        SECTION("From i64 vector", "[value][array]") {
            std::vector<i64> ints = {1, 2, 3, 4};
            Value v(ints);

            REQUIRE(v.Length() == 4);
            REQUIRE(v.AsInt(0) == 1);
            REQUIRE(v.AsInt(1) == 2);
            REQUIRE(v.AsInt(2) == 3);
            REQUIRE(v.AsInt(3) == 4);
        }

        SECTION("From f64 span", "[value][array]") {
            std::vector floats = {1.1, 2.2, 3.3};
            Value v(std::span {floats});

            REQUIRE(v.Length() == 3);
            REQUIRE(v.AsFloat(0) == Approx(1.1));
            REQUIRE(v.AsFloat(1) == Approx(2.2));
            REQUIRE(v.AsFloat(2) == Approx(3.3));
        }

        SECTION("From u64 array", "[value][array]") {
            std::array<u64, 6> uints = {5, 10, 15, 20, 25, 30};
            Value v(uints);

            REQUIRE(v.Length() == 6);
            REQUIRE(v.AsInt(0) == 5);
            REQUIRE(v.AsInt(1) == 10);
            REQUIRE(v.AsInt(2) == 15);
            REQUIRE(v.AsInt(3) == 20);
            REQUIRE(v.AsInt(4) == 25);
            REQUIRE(v.AsInt(5) == 30);
        }

        SECTION("From empty vector", "[value][array]") {
            std::vector<i64> empty;
            REQUIRE(Value(empty).Length() == 0);
        }

        SECTION("Scalar Length is 1", "[value][array]") {
            Value v(42);

            REQUIRE(v.Length() == 1);
            REQUIRE(v.ByteLength() == sizeof(Value::Data));
        }
    }

    SECTION("Misc") {
        constexpr auto i64_max = std::numeric_limits<i64>::max();
        constexpr auto i64_min = std::numeric_limits<i64>::min();
        constexpr auto u64_max = std::numeric_limits<u64>::max();

        SECTION("Large Integer", "[value][misc]") {
            Value v(i64_max);
            REQUIRE(v.AsInt() == i64_max);
        }

        SECTION("Negative Integer", "[value][misc]") {
            Value v(i64_min);
            REQUIRE(v.AsInt() == i64_min);
        }

        SECTION("Max Unsigned", "[value][misc]") {
            Value v(u64_max);
            REQUIRE(v.AsUint() == u64_max);
        }
    }
}
