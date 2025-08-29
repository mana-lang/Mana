#pragma once

#include <mana/vm/slice.hpp>

#include <mana/literals.hpp>
#include <mana/vm/opcode.hpp>

namespace hex {
namespace ml = mana::literals;

void EmitConstant(ml::i64 offset, ml::f64 constant);
void EmitConstant(ml::i64 offset, ml::i64 constant);
void EmitConstant(ml::i64 offset, ml::u64 constant);
void EmitConstant(ml::i64 offset, bool constant);

void EmitSimple(ml::i64 offset, mana::vm::Op op);

void PrintBytecode(const mana::vm::Slice& c);

}  // namespace hex
