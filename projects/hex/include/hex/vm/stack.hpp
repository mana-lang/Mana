#pragma once

#include <hex/core/logger.hpp>

#include <string_view>
#include <vector>

namespace hex {
template <typename T>
class Stack {
    std::vector<T> stack {};
    T*             top {nullptr};

public:
    using ValueType = T;

    Stack() {
        stack.reserve(128);
        top = stack.data();
    }

    void Reset() {
        top = stack.data();
    }

    void Push(T value) {
        if (top == &stack.back()) {
            stack.reserve(stack.capacity() * 2);
        }

        *top = value;
        ++top;

        LogTop("push:  {}");
    }

    T Pop() {
        if (top != &stack.front()) {
            --top;
        } else {
            Log->error("Attempted to pop from empty stack.");
            return 0.0;
        }

        return *top;
    }

    T ViewTop() const {
        if (top == &stack.front()) {
            Log->error("Attempted to read from empty stack");
            return 0.0;
        }

        return *(top - 1);
    }

    T* Top() const {
        if (top == &stack.front()) {
            Log->error("Attempted to read from empty stack");
            return nullptr;
        }

        return top - 1;
    }

    void LogTop(const std::string_view msg) const {
#ifdef HEX_DEBUG
        Log->debug(fmt::runtime(msg), ViewTop());
#endif
    }

    void Op_Add() {
        T rhs       = Pop();
        *(top - 1) += rhs;

        LogTop("add:   {}");
    }

    void Op_Sub() {
        T rhs       = Pop();
        *(top - 1) -= rhs;

        LogTop("sub:   {}");
    }

    void Op_Mul() {
        T rhs       = Pop();
        *(top - 1) *= rhs;

        LogTop("mul:   {}");
    }

    void Op_Div() {
        T rhs       = Pop();
        *(top - 1) /= rhs;

        LogTop("div:   {}");
    }
};
}  // namespace hex