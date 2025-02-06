#include <hex/slice.hpp>

namespace hex {
void Slice::Write(Op opcode) {
    code.push_back(static_cast<u8>(opcode));
}

void Slice::Write(Op opcode, const u8 byte) {
    code.push_back(static_cast<u8>(opcode));
    code.push_back(byte);
}

usize Slice::AddConstant(const Value value) {
    constants.push_back(value);

    return constants.size() - 1;
}

auto Slice::Code() const -> const std::vector<u8>& {
    return code;
}

auto Slice::Code() -> std::vector<u8>& {
    return code;
}

Value Slice::ConstantAt(const i64 idx) const {
    return constants[idx];
}

}  // namespace hex