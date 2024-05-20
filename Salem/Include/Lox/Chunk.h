#pragma once

#include <Lox/Common.h>
#include <Lox/Value.hpp>

#include <vector>

enum class OpCode {
    Op_Return,
};

class Chunk {
    std::vector<OpCode> code_;
    Values values_;

public:
    void Disassemble() const;
    void Push(OpCode code);
};
