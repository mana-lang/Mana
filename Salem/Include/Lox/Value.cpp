#include <Lox/Value.hpp>

void Values::Push(const Value value) {
    values_.push_back(value);
}
