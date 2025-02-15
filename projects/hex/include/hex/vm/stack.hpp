#pragma once

#include <hex/core/logger.hpp>

#include <string_view>
#include <vector>

namespace hex {
template <typename T>
class Stack {
    std::vector<T> values {};
    T*             top {nullptr};

public:
    using ValueType = T;

    Stack() {
        values.reserve(128);
        top = values.data();
    }

    void Reset() {
        top = values.data();
    }

    void Push(T value) {
        if (top == &values.back()) {
            values.reserve(values.capacity() * 2);
        }

        *top = value;
        ++top;

        LogTop("push:  {}");
    }

    T Pop() {
        if (top != &values.front()) {
            --top;
        } else {
            Log->error("Attempted to pop from empty stack.");
            return 0.0;
        }

        return *top;
    }

    T ViewTop() const {
        if (top == &values.front()) {
            Log->error("Attempted to read from empty stack");
            return 0.0;
        }

        return *(top - 1);
    }

    T* Top() const {
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
        T rhs = Pop();

        *(top - 1) += rhs;

        LogTop("add:   {}");
    }

    void Op_Sub() {
        T rhs = Pop();
        // *(top - 1) -= rhs;

        LogTop("sub:   {}");
    }

    void Op_Mul() {
        T rhs = Pop();
        // *(top - 1) *= rhs;

        LogTop("mul:   {}");
    }

    void Op_Div() {
        T rhs = Pop();
        // *(top - 1) /= rhs;

        LogTop("div:   {}");
    }

    void Op_Neg() {
        *(top - 1) *= -1;

        LogTop("neg:   {}");
    }
};
}  // namespace hex