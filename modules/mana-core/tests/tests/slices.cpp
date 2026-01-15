#include <catch2/catch_test_macros.hpp>

#include <mana/vm/hexe.hpp>
#include <mana/vm/value.hpp>

using namespace mana::vm;
using namespace mana::literals;

TEST_CASE("Slices") {
    SECTION("Constant pool indexing") {
        Hexe s;

        u64 a = s.AddConstant(42);
        u64 b = s.AddConstant(3.5);
        u64 c = s.AddConstant(true);

        REQUIRE(a == 0);
        REQUIRE(b == 1);
        REQUIRE(c == 2);

        REQUIRE(s.Constants().size() == 3);
        REQUIRE(s.Constants()[a].AsInt() == 42);
        REQUIRE(s.Constants()[b].AsFloat() == 3.5);
        REQUIRE(s.Constants()[c].AsBool() == true);
    }

    SECTION("Instruction sequencing") {
        Hexe s;

        // Each Push instruction references a Constant in the Constant Pool
        // Constant indices are encoded as 16-bit values, allowing up to 65535 constants
        // This means each Push instruction takes up 3 bytes

        // Add instructions add the two topmost values on the stack, so they take up 1 byte
        // Return returns the topmost value on the stack, so it takes up only 1 byte
        s.Write(Op::Push, s.AddConstant(123)); // 3
        s.Write(Op::Push, s.AddConstant(45));  // 6
        s.Write(Op::Push, s.AddConstant(678)); // 9
        s.Write(Op::Add);                      // 10
        s.Write(Op::Return);                   // 11

        const auto& instr = s.Instructions();
        REQUIRE(instr.size() == 11);

        REQUIRE(instr[0] == static_cast<u8>(Op::Push));
        REQUIRE(instr[3] == static_cast<u8>(Op::Push));
        REQUIRE(instr[6] == static_cast<u8>(Op::Push));
        REQUIRE(instr[9] == static_cast<u8>(Op::Add));
        REQUIRE(instr[10] == static_cast<u8>(Op::Return));

        auto read_u16_be = [&instr](const u64 i) {
            return static_cast<u16>(instr[i]) << 8 | static_cast<u16>(instr[i + 1]);
        };

        REQUIRE(read_u16_be(1) == 0); // points to zeroeth pool element
        REQUIRE(read_u16_be(4) == 1);
        REQUIRE(read_u16_be(7) == 2);

        REQUIRE(s.Constants()[0] == 123);
        REQUIRE(s.Constants()[1] == 45);
        REQUIRE(s.Constants()[2] == 678);
    }

    SECTION("Serialization") {
        Hexe s;

        s.Write(Op::Push, s.AddConstant(10));
        s.Write(Op::Push, s.AddConstant(20));
        s.Write(Op::Add);
        s.Write(Op::Return);

        ByteCode bytes;
        REQUIRE_NOTHROW(bytes = s.Serialize());
        REQUIRE_FALSE(bytes.empty());

        Hexe d;
        REQUIRE_NOTHROW(d.Deserialize(bytes));

        REQUIRE(d.Constants().size() == s.Constants().size());
        REQUIRE(d.Instructions().size() == s.Instructions().size());

        for (u64 i = 0; i < s.Constants().size(); ++i) {
            REQUIRE(d.Constants()[i] == s.Constants()[i]);
        }

        for (u64 i = 0; i < s.Instructions().size(); ++i) {
            REQUIRE(d.Instructions()[i] == s.Instructions()[i]);
        }
    }

    SECTION("Empty slices") {
        Hexe s;

        REQUIRE(s.Instructions().empty());
        REQUIRE(s.Constants().empty());

        ByteCode bytes;
        REQUIRE_NOTHROW(bytes = s.Serialize());
        REQUIRE(bytes.empty());

        Hexe d;
        REQUIRE_NOTHROW(d.Deserialize(bytes));

        REQUIRE(d.Instructions().empty());
        REQUIRE(d.Constants().empty());
    }
}
