#include <hex/vm/stack.hpp>

namespace mana::vm {}  // namespace mana::vm

hex::Stack::Stack() {
    values.reserve(128);
    top = values.data();
}

void hex::Stack::Reset() {
    top = values.data();
}

void hex::Stack::Push(const mvm::Value value) {
    if (top == &values.back()) {
        values.reserve(values.capacity() * 2);
    }

    *top = value;
    ++top;

    LogTop("push:  {}");
}

mana::vm::Value hex::Stack::Pop() {
    if (top != &values.front()) {
        --top;
    } else {
        Log->error("Attempted to pop from empty stack.");
        return 0.0;
    }

    return *top;
}

mana::vm::Value hex::Stack::ViewTop() const {
    if (top == &values.front()) {
        Log->error("Attempted to read from empty stack");
        return 0.0;
    }

    return *(top - 1);
}

mana::vm::Value* hex::Stack::Top() const {
    if (top == &values.front()) {
        Log->error("Attempted to read from empty stack");
        return nullptr;
    }

    return top - 1;
}

void hex::Stack::LogTop(const std::string_view msg) const {
#ifdef HEX_DEBUG
    Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
#endif
}

void hex::Stack::LogTopBool(const std::string_view msg) const {
#ifdef HEX_DEBUG
    Log->debug(fmt::runtime(msg), ViewTop().AsBool());
#endif
}

void hex::Stack::Op_Add() {
    *(top - 1) += Pop();

    LogTop("add:   {}");
}

void hex::Stack::Op_Sub() {
    *(top - 1) -= Pop();

    LogTop("sub:   {}");
}

void hex::Stack::Op_Mul() {
    *(top - 1) *= Pop();

    LogTop("mul:   {}");
}

void hex::Stack::Op_Div() {
    *(top - 1) /= Pop();

    LogTop("div:   {}");
}

void hex::Stack::Op_Neg() {
    *(top - 1) *= -1;

    LogTop("neg:   {}");
}

void hex::Stack::Op_CmpGreater() {
    Push(Pop() > Pop());

    LogTopBool("cmp_greater: {}");
}

void hex::Stack::Op_CmpLesser() {
    Push(Pop() < Pop());

    LogTopBool("cmp_lesser: {}");
}