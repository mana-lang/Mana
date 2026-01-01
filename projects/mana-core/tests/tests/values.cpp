#include <catch2/catch_test_macros.hpp>
#include <mana/vm/value.hpp>

#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace mana;
using namespace mana::vm;

constexpr f64 float_tolerance = 0.0001;

TEST_CASE("Values") {
    constexpr i64 INT = 42;
    constexpr u64 UINT = 83;
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

        Value icmp {i64{42}};
        Value fcmp {54.3};

        REQUIRE(i == icmp);
        REQUIRE(u > fcmp);
        REQUIRE(f <= fcmp);
        REQUIRE(u >= i);
    }
}