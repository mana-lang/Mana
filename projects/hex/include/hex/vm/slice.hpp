#pragma once

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

#include <vector>

namespace hex {
using namespace mana::literals;
using namespace mana::vm;

using Value = f64;

class Slice {
public:
    void Write(Op opcode);
    void Write(Op opcode, u8 byte);

    usize AddConstant(Value value);

    auto Code() const -> const std::vector<u8>&;
    auto Code() -> std::vector<u8>&;
    auto Constants() const -> const std::vector<Value>&;

    HEX_NODISCARD Value ConstantAt(i64 index) const;

    HEX_NODISCARD auto Serialize() const -> std::vector<u8>;
    void               Deserialize(const std::vector<u8>& bytes);

private:
    std::vector<u8>    code;
    std::vector<Value> constants;
};

}  // namespace hex
