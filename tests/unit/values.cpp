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

            constexpr std::string_view str = "this string is longer than the other one, is it constructed properly?";
            constexpr auto rdiv_len        = (str.length() + sizeof(Value::Data) - 1) / sizeof(Value::Data);
            Value v3(str);
            REQUIRE(v3.Type() == Value::Data::Type::String);
            REQUIRE(v3.AsString() == str);
            REQUIRE(v3.Length() == rdiv_len);
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
                Value a(-1ull);
                REQUIRE(a.AsUint() == -1ull);
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
        SECTION("Copy scalar", "[value][lifecycle]") {
            Value a(42);
            Value b(a);
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Copy string") {
            constexpr auto str = "this is a string that should be preserved";
            Value a(str);
            Value b(a);

            REQUIRE(b.AsString() == str);
            REQUIRE(a.AsString() == b.AsString());
        }

        SECTION("Copy assignment with scalar Value", "[value][lifecycle]") {
            Value a(42);
            Value b(0);
            b = a;
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == b.AsInt());
        }

        SECTION("Copy assignment with string Value") {
            Value a(std::string_view("hello"));
            Value b(99);
            b = a;

            REQUIRE(b.AsString() == "hello");
            REQUIRE(a.AsString() == b.AsString());
        }

        SECTION("Self-assignment (copy)", "[value][lifecycle]") {
            Value a(42);
            const Value& ref = a;

            a = ref;
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Move scalar", "[value][lifecycle]") {
            Value a(42);
            Value b(std::move(a));
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move string") {
            Value a(std::string_view("hello world"));
            Value b(std::move(a));

            REQUIRE(b.AsString() == "hello world");
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move assignment with scalar", "[value][lifecycle]") {
            Value a(99);
            Value b(0);

            b = std::move(a);
            REQUIRE(b.AsInt() == 99);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move assignment with string") {
            Value a(std::string_view("hello"));
            Value b(99);

            b = std::move(a);
            REQUIRE(b.AsString() == "hello");
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Self-assignment (move)", "[value][lifecycle]") {
            Value a(42);
            a = std::move(a);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Overwrite string with scalar", "[value][lifecycle]") {
            Value a(
                "hello everybody, this string is pretty long i think, do you all agree? perhaps it could even be a little longer"
            );
            REQUIRE(a.Type() == Value::Data::Type::String);

            a = Value(42);
            REQUIRE(a.Type() == Value::Data::Type::Int64);
            REQUIRE(a.Length() == 1);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Overwrite scalar with string", "[value][lifecycle]") {
            Value a(42);

            const auto str =
                "hello everyone, this string is also pretty long i think, but it's a different one this time";
            a = Value(str);
            REQUIRE(a.Type() == Value::Data::Type::String);
            REQUIRE(a.AsString() == str);
        }
        SECTION("Overwrite string with longer string", "[value][lifecycle]") {
            Value a("short");
            constexpr auto str = "this string is kinda shorter than before but long enough";

            a = Value(str);
            REQUIRE(a.AsString() == str);

            // direct assignment
            Value b("another short");
            b = str;
            REQUIRE(b.AsString() == str);
        }

        SECTION("Copy default-constructed Value", "[value][lifecycle]") {
            Value a;
            Value b(a);
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move default-constructed Value", "[value][lifecycle]") {
            Value a;
            Value b(std::move(a));

            REQUIRE(b.Type() == Value::Data::Type::Invalid);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assign to default-constructed Value", "[value][lifecycle]") {
            Value a;
            a = Value(42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Assign default to valid Value", "[value][lifecycle]") {
            Value a(42);
            Value b;
            a = b;
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Overwrite string with longer string") {
            Value a(std::string_view("short"));
            a = Value(std::string_view("this is a much longer string than before"));
            REQUIRE(a.AsString() == "this is a much longer string than before");
        }

        SECTION("Copy default-constructed Value") {
            Value a;
            Value b(a);
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move default-constructed Value") {
            Value a;
            Value b(std::move(a));
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assign to default-constructed Value") {
            Value a;
            a = Value(42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Assign default to valid Value") {
            Value a(42);
            Value b;
            a = b;
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }
        SECTION("Double move") {
            Value a(42);
            Value b(std::move(a));
            REQUIRE(a.Type() == Value::Data::Type::Invalid);

            // shouldn't crash
            Value c(std::move(a));
            REQUIRE(c.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assignment via pointer indirection") {
            Value a(42);
            Value* p = &a;
            Value b(*p);
            REQUIRE(a.AsInt() == b.AsInt());
        }

        SECTION("Self-assignment via pointer indirection") {
            Value a(42);
            Value* p = &a;
            a        = *p;
            REQUIRE(a.AsInt() == 42);
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
            REQUIRE(v.NumBytes() == sizeof(Value::Data));
        }
    }

    SECTION("Edge Cases", "[value][edge]") {
        SECTION("operator==, [value][edge]") {
            SECTION("Cross-type equality returns false", "[value][edge]") {
                // this is something we'll have to fix in the future
                Value a(1);
                Value b(1ull);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Int vs Float equality returns false", "[value][edge]") {
                Value a(1);
                Value b(1.0);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Int vs Bool equality returns false", "[value][edge]") {
                Value a(1);
                Value b(true);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Array equality always returns false", "[value][edge]") {
                std::vector data = {1, 2, 3};
                Value a(data);
                Value b(data);
                // Documented limitation: array comparison not implemented
                REQUIRE_FALSE(a == b);
            }

            SECTION("Bool equality", "[value][edge]") {
                REQUIRE(Value(true) == Value(true));
                REQUIRE(Value(false) == Value(false));
                REQUIRE_FALSE(Value(true) == Value(false));
            }

            SECTION("Float equality", "[value][edge]") {
                REQUIRE(Value(3.14) == Value(3.14));
                REQUIRE_FALSE(Value(3.14) == Value(2.71));
            }

            SECTION("Unsigned equality", "[value][edge]") {
                Value a(42ull);
                Value b(42ull);
                Value c(99ull);
                REQUIRE(a == b);
                REQUIRE_FALSE(a == c);
            }

            SECTION("String equality", "[value][edge]") {
                REQUIRE(Value("hello") == Value("hello"));
                REQUIRE_FALSE(Value("hello") == Value("world"));
            }
        }

        SECTION("String", "[value][edge][str]") {
            SECTION("String exactly sizeof(Data) bytes (8)") {
                Value v("12345678");
                REQUIRE(v.Type() == Value::Data::Type::String);
                REQUIRE(v.AsString() == "12345678");
                REQUIRE(v.Length() == 1);
            }

            SECTION("String of 9 bytes — crosses Data boundary") {
                Value v("123456789");
                REQUIRE(v.AsString() == "123456789");
                REQUIRE(v.Length() == 2);
            }

            SECTION("String with embedded null bytes") {
                std::string_view sv("ab\0cd", 5);
                Value v(sv);
                REQUIRE(v.NumBytes() == 5);
                REQUIRE(v.AsString() == sv);

                const auto s = "ab\0cd";
                Value v2(s);
                REQUIRE(v2.NumBytes() == 2);
                REQUIRE(v2.AsString() == s);
            }

            SECTION("Long string (1000 chars)") {
                std::string long_str(1000, 'x');
                Value v(long_str);
                REQUIRE(v.AsString() == long_str);
                REQUIRE(v.NumBytes() == 1000);
            }

            SECTION("AsString on non-string throws") {
                Value v(42);
                REQUIRE_THROWS_AS(v.AsString(), std::runtime_error);
            }
        }

        SECTION("Misc, [value][edge][misc]") {
            constexpr auto i64_max = std::numeric_limits<i64>::max();
            constexpr auto i64_min = std::numeric_limits<i64>::min();

            constexpr auto u64_max = std::numeric_limits<u64>::max();

            SECTION("Large Integer", "[value][edge][misc]") {
                Value v(i64_max);
                REQUIRE(v.AsInt() == i64_max);
            }

            SECTION("Negative Integer", "[value][edge][misc]") {
                Value v(i64_min);
                REQUIRE(v.AsInt() == i64_min);
            }

            SECTION("Max Unsigned", "[value][edge][misc]") {
                Value v(u64_max);
                REQUIRE(v.AsUint() == u64_max);
            }
        }
    }
}
