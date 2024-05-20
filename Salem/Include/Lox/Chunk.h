#pragma once

#include <Lox/Common.h>
#include <Lox/Value.hpp>

#include <vector>

enum class OpCode {
    Return,
    Constant,
};

class Chunk {
    std::vector<OpCode> code_;
    Values values_;

public:
    void Disassemble() const;
    void Push(OpCode code);
    void PushValue(Value value);
};
