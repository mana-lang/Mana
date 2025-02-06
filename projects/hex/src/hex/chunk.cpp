#include <hex/chunk.hpp>

namespace hex {
void Chunk::Write(Op opcode) {
    code.push_back(static_cast<u8>(opcode));
}

void Chunk::Write(Op opcode, const u8 byte) {
    code.push_back(static_cast<u8>(opcode));
    code.push_back(byte);
}

usize Chunk::AddConstant(const Value value) {
    constants.push_back(value);

    return constants.size() - 1;
}

auto Chunk::Code() const -> const std::vector<u8>& {
    return code;
}

Value Chunk::ConstantAt(const i64 idx) const {
    return constants[idx];
}

}  // namespace hex