#pragma once

#include <hex/core/logger.hpp>

#include <mana/vm/value.hpp>
#include <string_view>
#include <vector>

namespace hex {
namespace mvm = mana::vm;

class Stack {
    std::vector<mvm::Value> values {};
    mvm::Value*             top {nullptr};

public:
    Stack() {
        values.reserve(128);
        top = values.data();
    }

    void Reset() {
        top = values.data();
    }

    void Push(const mvm::Value value) {
        if (top == &values.back()) {
            values.reserve(values.capacity() * 2);
        }

        *top = value;
        ++top;

        LogTop("push:  {}");
    }

    mvm::Value Pop() {
        if (top != &values.front()) {
            --top;
        } else {
            Log->error("Attempted to pop from empty stack.");
            return 0.0;
        }

        return *top;
    }

    mvm::Value ViewTop() const {
        if (top == &values.front()) {
            Log->error("Attempted to read from empty stack");
            return 0.0;
        }

        return *(top - 1);
    }

    mvm::Value* Top() const {
        if (top == &values.front()) {
            Log->error("Attempted to read from empty stack");
            return nullptr;
        }

        return top - 1;
    }

    void LogTop(const std::string_view msg) const {
#ifdef HEX_DEBUG
        Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
#endif
    }

    void Op_Add() {
        *(top - 1) += Pop();

        LogTop("add:   {}");
    }

    void Op_Sub() {
        *(top - 1) -= Pop();

        LogTop("sub:   {}");
    }

    void Op_Mul() {
        *(top - 1) *= Pop();

        LogTop("mul:   {}");
    }

    void Op_Div() {
        *(top - 1) /= Pop();

        LogTop("div:   {}");
    }

    void Op_Neg() {
        *(top - 1) *= -1;

        LogTop("neg:   {}");
    }
};
}  // namespace hex