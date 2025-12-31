#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

using namespace mana;
using namespace mana::vm;
using namespace mana::literals;

TEST_CASE("Bytecode", "[serde][bytecode]") {
    SECTION("Serializing") {
        SECTION("Slice") {
            Slice slice;

            slice.Write(Op::Push, slice.AddConstant(23.65));
            slice.Write(Op::Push, slice.AddConstant(123.4));
            slice.Write(Op::Add);
            slice.Write(Op::Return);

            const auto bytes = slice.Serialize();

            Slice deserialized;
            deserialized.Deserialize(bytes);

            REQUIRE(deserialized.Constants().size() == slice.Constants().size());
            for (u64 i = 0; i < slice.Constants().size(); ++i) {
                CHECK(deserialized.Constants()[i] == slice.Constants()[i]);
            }

            REQUIRE(deserialized.Instructions().size() == slice.Instructions().size());
            for (u64 i = 0; i < slice.Instructions().size(); ++i) {
                CHECK(deserialized.Instructions()[i] == slice.Instructions()[i]);
            }

            REQUIRE(deserialized.Constants()[0].GetType() == Float64);
            CHECK_THAT(deserialized.Constants()[0].AsFloat(),
                       Catch::Matchers::WithinAbs(23.65, 0.001));
        }
    }
}