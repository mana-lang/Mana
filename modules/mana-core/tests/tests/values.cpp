#include <mana/hexe/value.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace mana;
using namespace mana::hexe;

constexpr f64 float_tolerance = 0.0001;

TEST_CASE("Values") {
    constexpr i64 INT   = 42;
    constexpr u64 UINT  = 83;
    constexpr f64 FLOAT = 3.5;

    Value i {INT};
    Value u {UINT};
    Value f {FLOAT};
    Value b {true};

    SECTION("Construction and type tagging") {
        REQUIRE(i.GetType() == Int64);
        REQUIRE(u.GetType() == Uint64);
        REQUIRE(f.GetType() == Float64);
        REQUIRE(b.GetType() == Bool);

        REQUIRE(i.Length() == 1);
        REQUIRE(u.Length() == 1);
        REQUIRE(f.Length() == 1);
        REQUIRE(b.Length() == 1);
    }

    SECTION("Conversions") {
        REQUIRE_THAT(static_cast<f64>(INT), Catch::Matchers::WithinAbs(i.AsFloat(), float_tolerance));
        REQUIRE(i.AsBool());
        REQUIRE(i.AsUint() == static_cast<u64>(INT));

        REQUIRE_THAT(static_cast<f64>(UINT), Catch::Matchers::WithinAbs(u.AsFloat(), float_tolerance));
        REQUIRE(u.AsBool());
        REQUIRE(u.AsInt() == static_cast<i64>(UINT));

        REQUIRE(f.AsBool());
        REQUIRE(f.AsInt() == static_cast<i64>(FLOAT));
        REQUIRE(f.AsUint() == static_cast<u64>(FLOAT));

        REQUIRE(b.AsInt() == 1);
        REQUIRE(b.AsUint() == 1);
        REQUIRE(b.AsFloat() == 1.0);
    }

    SECTION("Equality and comparison") {
        REQUIRE(i != u);
        REQUIRE(i < u);

        Value icmp = 42;
        Value fcmp = 54.3;

        REQUIRE(i == icmp);
        REQUIRE(u > fcmp);
        REQUIRE(f <= fcmp);
        REQUIRE(u >= i);
    }

    SECTION("Arithmetic operations") {
        Value x = 23;
        Value y = 41.53;

        x += i;
        REQUIRE(x.AsInt() == 65);

        x -= 2;
        REQUIRE(x.AsInt() == 63);

        x *= 2;
        REQUIRE(x.AsInt() == 126);

        x /= 3;
        REQUIRE(x.AsInt() == 42);
    }

    SECTION("Copy semantics") {
        Value x = 71;
        Value y = x;

        REQUIRE(x == y);

        x += 1;
        REQUIRE(x.AsInt() == 72);
        REQUIRE(y.AsInt() == 71);
    }

    SECTION("Move semantics") {
        Value x = 12;
        Value y = std::move(x);

        REQUIRE(x.GetType() == Invalid);
        REQUIRE(x.Length() == 0);

        REQUIRE(y.AsInt() == 12);
        REQUIRE(y.GetType() == Int64);
        REQUIRE(y.Length() == 1);
    }

    SECTION("Array construction") {
        std::vector<i64> v {8, 12, 31, 24, 15};
        Value x {v};

        REQUIRE(x.GetType() == Int64);
        REQUIRE(x.Length() == v.size());

        for (u64 k = 0; k < v.size(); ++k) {
            REQUIRE(x.BitCasted(k) == std::bit_cast<u64>(v[k]));
        }
    }
}
