#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <hexec/value.hpp>

#include <vector>
#include <span>

using namespace hexec;
using Catch::Approx;

// ReSharper disable once CppDFATimeOver
TEST_CASE("VM Generic const Value Type", "[const Value]") {
    constexpr auto i64_max = std::numeric_limits<i64>::max();
    constexpr auto i64_min = std::numeric_limits<i64>::min();

    constexpr auto u64_max = std::numeric_limits<u64>::max();
    constexpr auto u64_min = std::numeric_limits<u64>::min();

    SECTION("Construction") {
        SECTION("Default construction", "[const Value]") {
            const Value v;
        }

        SECTION("From i64", "[const Value]") {
            const Value v(static_cast<i64>(42));
            REQUIRE(v.Type() == Value::Data::Type::Int64);
            REQUIRE(v.AsInt() == 42);
        }


        SECTION("From i32", "[const Value]") {
            const Value v(static_cast<i32>(-7));
            REQUIRE(v.Type() == Value::Data::Type::Int64);
            REQUIRE(v.AsInt() == -7);
        }

        SECTION("From u64", "[const Value]") {
            const Value v(static_cast<u64>(100));
            REQUIRE(v.Type() == Value::Data::Type::Uint64);
            REQUIRE(v.AsUint() == 100);
        }

        SECTION("From u32", "[const Value]") {
            const Value v(static_cast<u32>(50));
            REQUIRE(v.Type() == Value::Data::Type::Uint64);
            REQUIRE(v.AsUint() == 50);
        }

        SECTION("From f64", "[const Value]") {
            const Value v(3.14);
            REQUIRE(v.Type() == Value::Data::Type::Float64);
            REQUIRE(v.AsFloat() == Approx(3.14));
        }

        SECTION("From bool", "[const Value]") {
            const Value t(true), f(false);
            REQUIRE(t.Type() == Value::Data::Type::Bool);
            REQUIRE(t.AsBool() == true);
            REQUIRE(f.AsBool() == false);
        }

        SECTION("From string", "[const Value]") {
            const Value v(std::string_view("hello"));
            REQUIRE(v.Type() == Value::Data::Type::String);
            REQUIRE(v.AsString() == "hello");

            const Value v2("world");
            REQUIRE(v2.Type() == Value::Data::Type::String);
            REQUIRE(v2.AsString() == "world");

            constexpr std::string_view str = "this string is longer than the other one, is it constructed properly?";
            constexpr auto rdiv_len        = (str.length() + sizeof(Value::Data) - 1) / sizeof(Value::Data);
            const Value v3(str);
            REQUIRE(v3.Type() == Value::Data::Type::String);
            REQUIRE(v3.AsString() == str);
            REQUIRE(v3.Length() == rdiv_len);
        }

        SECTION("From empty string", "[const Value]") {
            const Value v(std::string_view(""));
            REQUIRE(v.Type() == Value::Data::Type::String);
            REQUIRE(v.AsString() == "");

            const Value v2("");
            REQUIRE(v2.Type() == Value::Data::Type::String);
            REQUIRE(v2.AsString() == "");
        }
    }

    SECTION("Arithmetic") {
        SECTION("Integers") {
            SECTION("Addition", "[const Value][arithmetic]") {
                const Value a(10), b(3);
                REQUIRE((a + b).AsInt() == 13);
            }

            SECTION("Subtraction", "[const Value][arithmetic]") {
                const Value a(10), b(3);
                REQUIRE((a - b).AsInt() == 7);
            }

            SECTION("Multiplication", "[const Value][arithmetic]") {
                const Value a(10), b(3);
                REQUIRE((a * b).AsInt() == 30);
            }

            SECTION("Division", "[const Value][arithmetic]") {
                const Value a(10), b(3);
                REQUIRE((a / b).AsInt() == 3);
            }

            SECTION("Modulo", "[const Value][arithmetic]") {
                const Value a(10), b(3);
                REQUIRE((a % b).AsInt() == 1);
            }

            SECTION("Negation", "[const Value][arithmetic]") {
                const Value a(5);
                REQUIRE((-a).AsInt() == -5);
            }

            SECTION("Double Negation", "[const Value][arithmetic]") {
                const Value a(-42);
                REQUIRE((-a).AsInt() == 42);
            }

            SECTION("Unsigned Underflow", "[const Value][arithmetic]") {
                const Value a(-1ull);
                REQUIRE(a.AsUint() == -1ull);
            }
        }

        SECTION("Floats") {
            SECTION("Addition", "[const Value][arithmetic]") {
                const Value a(2.5), b(4.0);
                REQUIRE((a + b).AsFloat() == Approx(6.5));
            }

            SECTION("Subtraction", "[const Value][arithmetic]") {
                const Value a(2.5), b(4.0);
                REQUIRE((a - b).AsFloat() == Approx(-1.5));
            }

            SECTION("Multiplication", "[const Value][arithmetic]") {
                const Value a(2.5), b(4.0);
                REQUIRE((a * b).AsFloat() == Approx(10.0));
            }

            SECTION("Division", "[const Value][arithmetic]") {
                const Value a(2.5), b(4.0);
                REQUIRE((a / b).AsFloat() == Approx(0.625));
            }

            SECTION("Modulo", "[const Value][arithmetic]") {
                const Value a(7.53), b(2.38);
                REQUIRE((a % b).AsFloat() == Approx(0.39));
            }

            SECTION("Negation", "[const Value][arithmetic]") {
                const Value a(2.5);
                REQUIRE((-a).AsFloat() == Approx(-2.5));
            }

            SECTION("Double Negation", "[const Value][arithmetic]") {
                const Value a(-2.5);
                REQUIRE((-a).AsFloat() == Approx(2.5));
            }
        }
    }
    SECTION("Compound Assignment") {
        SECTION("Add", "[const Value][compound]") {
            Value a(5);
            a += Value(3);
            REQUIRE(a.AsInt() == 8);
        }

        SECTION("Subtract", "[const Value][compound]") {
            Value a(10);
            a -= Value(4);
            REQUIRE(a.AsInt() == 6);
        }

        SECTION("Multiply", "[const Value][compound]") {
            Value a(3);
            a *= Value(7);
            REQUIRE(a.AsInt() == 21);
        }

        SECTION("Divide", "[const Value][compound]") {
            Value a(20);
            a /= Value(4);
            REQUIRE(a.AsInt() == 5);
        }

        SECTION("Modulo", "[const Value][compound]") {
            Value a(10);
            a %= Value(3);
            REQUIRE(a.AsInt() == 1);
        }
    }

    SECTION("Comparison") {
        SECTION("Equality", "[const Value][comparison]") {
            const Value a(10), b(10), c(20);
            REQUIRE(a == b);
            REQUIRE_FALSE(a == c);
        }

        SECTION("Less Than", "[const Value][comparison]") {
            const Value a(10), b(20);
            REQUIRE(a < b);
            REQUIRE_FALSE(b < a);
        }

        SECTION("Greater Than", "[const Value][comparison]") {
            const Value a(10), b(20);
            REQUIRE(b > a);
            REQUIRE_FALSE(a > b);
        }

        SECTION("Less Than or Equal", "[const Value][comparison]") {
            const Value a(10), b(10), c(20);
            REQUIRE(a <= b);
            REQUIRE(a <= c);
            REQUIRE_FALSE(c <= a);
        }

        SECTION("Greater Than or Equal", "[const Value][comparison]") {
            const Value a(10), b(10), c(20);
            REQUIRE(a >= b);
            REQUIRE(c >= a);
            REQUIRE_FALSE(a >= c);
        }

        SECTION("Logical not", "[const Value][comparison]") {
            const Value t(true), f(false);
            REQUIRE(!f);
            REQUIRE_FALSE(!t);
        }
    }

    SECTION("Type Dispatch") {
        SECTION("From int to float", "[const Value][dispatch]") {
            const Value v(static_cast<i64>(42));
            REQUIRE(v.AsFloat() == Approx(42.0));
        }

        SECTION("From float to int (truncation)", "[const Value][dispatch]") {
            const Value v(3.7);
            REQUIRE(v.AsInt() == 3);
        }

        SECTION("From bool to int", "[const Value][dispatch]") {
            REQUIRE(Value(true).AsInt() == 1);
            REQUIRE(Value(false).AsInt() == 0);
        }

        SECTION("From bool to float", "[const Value][dispatch]") {
            REQUIRE(Value(true).AsFloat() == Approx(1.0));
            REQUIRE(Value(false).AsFloat() == Approx(0.0));
        }
    }

    SECTION("Lifecycle") {
        SECTION("Copy scalar", "[const Value][lifecycle]") {
            const Value a(42);
            const Value b(a);
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Copy string") {
            constexpr auto str = "this is a string that should be preserved";
            const Value a(str);
            const Value b(a);

            REQUIRE(b.AsString() == str);
            REQUIRE(a.AsString() == b.AsString());
        }

        SECTION("Copy assignment with scalar const Value", "[const Value][lifecycle]") {
            const Value a(42);
            Value b(0);
            b = a;
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.AsInt() == b.AsInt());
        }

        SECTION("Copy assignment with string const Value") {
            const Value a(std::string_view("hello"));
            Value b(99);
            b = a;

            REQUIRE(b.AsString() == "hello");
            REQUIRE(a.AsString() == b.AsString());
        }

        SECTION("Self-assignment (copy)", "[const Value][lifecycle]") {
            Value a(42);
            const Value& ref = a;

            a = ref;
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Move scalar", "[const Value][lifecycle]") {
            Value a(42);
            const Value b(std::move(a));
            REQUIRE(b.AsInt() == 42);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move string") {
            Value a(std::string_view("hello world"));
            const Value b(std::move(a));

            REQUIRE(b.AsString() == "hello world");
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move assignment with scalar", "[const Value][lifecycle]") {
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

        SECTION("Self-assignment (move)", "[const Value][lifecycle]") {
            Value a(42);
            a = std::move(a);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Overwrite string with scalar", "[const Value][lifecycle]") {
            Value a(
                "hello everybody, this string is pretty long i think, do you all agree? perhaps it could even be a little longer"
            );
            REQUIRE(a.Type() == Value::Data::Type::String);

            a = Value(42);
            REQUIRE(a.Type() == Value::Data::Type::Int64);
            REQUIRE(a.Length() == 1);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Overwrite scalar with string", "[const Value][lifecycle]") {
            Value a(42);

            const auto str =
                "hello everyone, this string is also pretty long i think, but it's a different one this time";
            a = Value(str);
            REQUIRE(a.Type() == Value::Data::Type::String);
            REQUIRE(a.AsString() == str);
        }
        SECTION("Overwrite string with longer string", "[const Value][lifecycle]") {
            Value a("short");
            constexpr auto str = "this string is kinda shorter than before but long enough";

            a = Value(str);
            REQUIRE(a.AsString() == str);

            // direct assignment
            Value b("another short");
            b = str;
            REQUIRE(b.AsString() == str);
        }

        SECTION("Copy default-constructed const Value", "[const Value][lifecycle]") {
            const Value a;
            const Value b(a);
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move default-constructed const Value", "[const Value][lifecycle]") {
            const Value a;
            const Value b(std::move(a));

            REQUIRE(b.Type() == Value::Data::Type::Invalid);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assign to default-constructed const Value", "[const Value][lifecycle]") {
            Value a;
            a = Value(42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Assign default to valid const Value", "[const Value][lifecycle]") {
            Value a(42);
            const Value b;
            a = b;
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Overwrite string with longer string") {
            Value a(std::string_view("short"));
            a = Value(std::string_view("this is a much longer string than before"));
            REQUIRE(a.AsString() == "this is a much longer string than before");
        }

        SECTION("Copy default-constructed const Value") {
            const Value a;
            const Value b(a);
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Move default-constructed const Value") {
            const Value a;
            const Value b(std::move(a));
            REQUIRE(b.Type() == Value::Data::Type::Invalid);
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assign to default-constructed const Value") {
            Value a;
            a = Value(42);
            REQUIRE(a.AsInt() == 42);
        }

        SECTION("Assign default to valid const Value") {
            Value a(42);
            const Value b;
            a = b;
            REQUIRE(a.Type() == Value::Data::Type::Invalid);
        }
        SECTION("Double move") {
            Value a(42);
            const Value b(std::move(a));
            REQUIRE(a.Type() == Value::Data::Type::Invalid);

            // shouldn't crash
            const Value c(std::move(a));
            REQUIRE(c.Type() == Value::Data::Type::Invalid);
        }

        SECTION("Assignment via pointer indirection") {
            const Value a(42);
            const Value* p = &a;
            const Value b(*p);
            REQUIRE(a.AsInt() == b.AsInt());
        }

        SECTION("Self-assignment via pointer indirection") {
            Value a(42);
            const Value* p = &a;

            a = *p;
            REQUIRE(a.AsInt() == 42);
        }
    }

    SECTION("Arrays") {
        SECTION("From i64 vector", "[const Value][array]") {
            std::vector<i64> ints = {1, 2, 3, 4};
            const Value v(ints);

            REQUIRE(v.Length() == 4);
            REQUIRE(v.AsInt(0) == 1);
            REQUIRE(v.AsInt(1) == 2);
            REQUIRE(v.AsInt(2) == 3);
            REQUIRE(v.AsInt(3) == 4);
        }

        SECTION("From f64 span", "[const Value][array]") {
            std::vector floats = {1.1, 2.2, 3.3};
            const Value v(std::span {floats});

            REQUIRE(v.Length() == 3);
            REQUIRE(v.AsFloat(0) == Approx(1.1));
            REQUIRE(v.AsFloat(1) == Approx(2.2));
            REQUIRE(v.AsFloat(2) == Approx(3.3));
        }

        SECTION("From u64 array", "[const Value][array]") {
            std::array<u64, 6> uints = {5, 10, 15, 20, 25, 30};
            const Value v(uints);

            REQUIRE(v.Length() == 6);
            REQUIRE(v.AsInt(0) == 5);
            REQUIRE(v.AsInt(1) == 10);
            REQUIRE(v.AsInt(2) == 15);
            REQUIRE(v.AsInt(3) == 20);
            REQUIRE(v.AsInt(4) == 25);
            REQUIRE(v.AsInt(5) == 30);
        }

        SECTION("From empty vector", "[const Value][array]") {
            std::vector<i64> empty;
            REQUIRE(Value(empty).Length() == 0);
        }

        SECTION("Scalar Length is 1", "[const Value][array]") {
            const Value v(42);

            REQUIRE(v.Length() == 1);
            REQUIRE(v.NumBytes() == sizeof(Value::Data));
        }
    }

    SECTION("Edge Cases", "[const Value][edge]") {
        SECTION("operator==, [const Value][edge][eq]") {
            SECTION("Cross-type equality returns false", "[const Value][edge][eq]") {
                // this is something we'll have to fix in the future
                const Value a(1);
                const Value b(1ull);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Int vs Float equality returns false", "[const Value][edge][eq]") {
                const Value a(1);
                const Value b(1.0);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Int vs Bool equality returns false", "[const Value][edge][eq]") {
                const Value a(1);
                const Value b(true);
                REQUIRE_FALSE(a == b);
            }

            SECTION("Array equality always returns false", "[const Value][edge][eq]") {
                std::vector data = {1, 2, 3};
                const Value a(data);
                const Value b(data);
                // Documented limitation: array comparison not implemented
                REQUIRE_FALSE(a == b);
            }

            SECTION("Bool equality", "[const Value][edge][eq]") {
                REQUIRE(Value(true) == Value(true));
                REQUIRE(Value(false) == Value(false));
                REQUIRE_FALSE(Value(true) == Value(false));
            }

            SECTION("Float equality", "[const Value][edge][eq]") {
                REQUIRE(Value(3.14) == Value(3.14));
                REQUIRE_FALSE(Value(3.14) == Value(2.71));
            }

            SECTION("Unsigned equality", "[const Value][edge][eq]") {
                const Value a(42ull);
                const Value b(42ull);
                const Value c(99ull);
                REQUIRE(a == b);
                REQUIRE_FALSE(a == c);
            }

            SECTION("String equality", "[const Value][edge][eq]") {
                REQUIRE(Value("hello") == Value("hello"));
                REQUIRE_FALSE(Value("hello") == Value("world"));
            }
        }

        SECTION("String", "[const Value][edge][str]") {
            SECTION("String exactly sizeof(Data) bytes (8)", "[const Value][edge][str]") {
                const Value v("12345678");
                REQUIRE(v.Type() == Value::Data::Type::String);
                REQUIRE(v.AsString() == "12345678");
                REQUIRE(v.Length() == 1);
            }

            SECTION("String of 9 bytes — crosses Data boundary", "[const Value][edge][str]") {
                const Value v("123456789");
                REQUIRE(v.AsString() == "123456789");
                REQUIRE(v.Length() == 2);
            }

            SECTION("String with embedded null bytes", "[const Value][edge][str]") {
                std::string_view sv("ab\0cd", 5);
                const Value v(sv);
                REQUIRE(v.NumBytes() == 5);
                REQUIRE(v.AsString() == sv);

                const auto s = "ab\0cd";
                const Value v2(s);
                REQUIRE(v2.NumBytes() == 2);
                REQUIRE(v2.AsString() == s);
            }

            SECTION("Long string (1000 chars)", "[const Value][edge][str]") {
                std::string long_str(1000, 'x');
                const Value v(long_str);
                REQUIRE(v.AsString() == long_str);
                REQUIRE(v.NumBytes() == 1000);
            }

            SECTION("AsString on non-string throws", "[const Value][edge][str]") {
                const Value v(42);
                REQUIRE_THROWS_AS(v.AsString(), std::runtime_error);
            }
        }

        SECTION("Type Dispatch", "[const Value][edge][type]") {
            SECTION("AsInt on large u64 — wraps", "[const Value][edge][type]") {
                const Value v(u64_max);
                REQUIRE(v.AsInt() == static_cast<i64>(u64_max));
            }

            SECTION("AsUint on negative i64 — wraps", "[const Value][edge][type]") {
                const Value v(static_cast<i64>(-1));
                REQUIRE(v.AsUint() == u64_max);
            }

            SECTION("AsBool on nonzero int", "[const Value][edge][type]") {
                const Value v(42);
                REQUIRE(v.AsBool() == true);
            }

            SECTION("AsBool on zero int", "[const Value][edge][type]") {
                const Value v(0);
                REQUIRE(v.AsBool() == false);
            }

            SECTION("AsBool on nonzero float", "[const Value][edge][type]") {
                const Value v(3.14);
                REQUIRE(v.AsBool() == true);
            }

            SECTION("AsBool on zero float", "[const Value][edge][type]") {
                const Value v(0.0);
                REQUIRE(v.AsBool() == false);
            }

            SECTION("AsFloat precision loss on large i64", "[const Value][edge][type]") {
                // 2^53 + 1 cannot be exactly represented in f64
                constexpr i64 big = (1LL << 53) + 1;
                const Value v(big);
                const f64 as_float = v.AsFloat();
                // The +1 gets lost in f64 precision
                REQUIRE(as_float == Approx(static_cast<f64>(1LL << 53)).epsilon(0.0001));
            }
        }

        SECTION("Misc, [const Value][edge][misc]") {
            SECTION("Large Integer", "[const Value][edge][misc]") {
                const Value v(i64_max);
                REQUIRE(v.AsInt() == i64_max);
            }

            SECTION("Negative Integer", "[const Value][edge][misc]") {
                const Value v(i64_min);
                REQUIRE(v.AsInt() == i64_min);
            }

            SECTION("Max Unsigned", "[const Value][edge][misc]") {
                const Value v(u64_max);
                REQUIRE(v.AsUint() == u64_max);
            }
        }
    }
}
