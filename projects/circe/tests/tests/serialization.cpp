#include <catch2/catch_test_macros.hpp>

#include <mana/literals.hpp>
#include <mana/vm/constant-pool.hpp>
#include <mana/vm/slice.hpp>

using namespace mana::vm;
using namespace mana::literals;

TEST_CASE("Bytecode", "[serde][bytecode]") {
    SECTION("Serializing") {
        SECTION("Constant Pool") {
            ConstantPool<f64> floats;
            floats.constants.push_back(493.343);

            const auto bytes = floats.GetSerialized();
            REQUIRE(bytes.size() == 8);

            const ByteCode control_bytes {0x73, 0x68, 0x91, 0xED, 0x7C, 0xD5, 0x7E, 0x40};
            for (i64 i = 0; i < bytes.size(); ++i) {
                REQUIRE(bytes[i] == control_bytes[i]);
            }

            ConstantPool<f64> new_floats;
            new_floats.Deserialize(bytes);
            REQUIRE(new_floats.constants.size() == 1);
            REQUIRE(new_floats.constants.back() == floats.constants.back());
        }

        SECTION("Slice") {
            Slice slice;

            slice.Write(Op::Push_Float, slice.AddConstant(23.65));
            slice.Write(Op::Push_Float, slice.AddConstant(123.4));
            slice.Write(Op::Add);
            slice.Write(Op::Return);

            const auto bytes = slice.Serialize();
            REQUIRE(
                bytes.size()
                == sizeof(u64) + sizeof(f64) * slice.FloatConstants().size() + slice.Instructions().size()
            );

            Slice deser_slice;
            deser_slice.Deserialize(bytes);

            REQUIRE(deser_slice.FloatConstants().size() == slice.FloatConstants().size());
            for (u64 i = 0; i < slice.FloatConstants().size(); ++i) {
                CHECK(deser_slice.FloatConstants()[i] == slice.FloatConstants()[i]);
            }

            REQUIRE(deser_slice.Instructions().size() == slice.Instructions().size());
            for (u64 i = 0; i < slice.Instructions().size(); ++i) {
                CHECK(deser_slice.Instructions()[i] == slice.Instructions()[i]);
            }
        }
    }
}