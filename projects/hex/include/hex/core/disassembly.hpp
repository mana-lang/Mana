#pragma once

#include <hex/vm/slice.hpp>

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

namespace hex {
using namespace mana::literals;
using namespace mana::vm;

void EmitConstant(i64 offset, Value constant);

void EmitSimple(i64 offset, Op op);

void PrintBytecode(const Slice& c);

}  // namespace hex
