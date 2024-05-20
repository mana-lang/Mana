#include <Lox/Chunk.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

void Chunk::Disassemble() const {
    for (const auto op : code_) {
        spdlog::warn("{}", magic_enum::enum_name(op));
    }
}

void Chunk::Push(OpCode code) {
    code_.emplace_back(code);
}

void Chunk::PushValue(Value value) {
    values_.Push(value);
}
