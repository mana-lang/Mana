#include <catch2/catch_test_macros.hpp>

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

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

            constexpr auto value_size = sizeof(Value::Data) + sizeof(Value::Type);
            REQUIRE(bytes.size() == sizeof(u64) + slice.Instructions().size() + value_size * slice.Constants().size());

            Slice deser_slice;
            deser_slice.Deserialize(bytes);

            REQUIRE(deser_slice.Constants().size() == slice.Constants().size());
            for (u64 i = 0; i < slice.Constants().size(); ++i) {
                CHECK(deser_slice.Constants()[i] == slice.Constants()[i]);
            }

            REQUIRE(deser_slice.Instructions().size() == slice.Instructions().size());
            for (u64 i = 0; i < slice.Instructions().size(); ++i) {
                CHECK(deser_slice.Instructions()[i] == slice.Instructions()[i]);
            }
        }
    }
}