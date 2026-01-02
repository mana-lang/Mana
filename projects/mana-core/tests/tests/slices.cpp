#include <catch2/catch_test_macros.hpp>

#include <mana/vm/slice.hpp>
#include <mana/vm/value.hpp>

using namespace mana::vm;
using namespace mana::literals;

TEST_CASE("Slices") {
    SECTION("Empty slice") {
        Slice s;

        REQUIRE(s.Instructions().empty());
        REQUIRE(s.Constants().empty());

        ByteCode bytes;
        REQUIRE_NOTHROW(bytes = s.Serialize());
        REQUIRE(bytes.empty());

        Slice d;
        REQUIRE_NOTHROW(d.Deserialize(bytes));

        REQUIRE(d.Instructions().empty());
        REQUIRE(d.Constants().empty());
    }
}