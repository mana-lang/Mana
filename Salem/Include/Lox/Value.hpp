#pragma once

#include <Lox/Common.h>

#include <vector>

using Value = double;

class Values {
    std::vector<Value> values_;
public:
    void Push(Value value);
};
