#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
VirtualMachine::VirtualMachine() {
    stack.reserve(256);
    stack_top = stack.data();
}

void VirtualMachine::ResetStack() {
    stack_top = stack.data();
}

void VirtualMachine::Push(const Value value) {
    if (stack_top == &stack.back()) {
        stack.reserve(stack.capacity() * 2);
    }

    *stack_top = value;
    ++stack_top;
}

Value VirtualMachine::Pop() {
    if (stack_top != &stack.front()) {
        --stack_top;
    } else {
        LogErr("Attempted to pop from empty stack.");
        return 0.0;
    }

    return *stack_top;
}

// clang-format off
#define BINARY_OP(op)              \
    {                              \
        Value a = Pop();           \
        *(stack_top - 1) op## = a; \
    }
// clang-format on

#ifdef HEX_DEBUG
#    define LOG_STACK_TOP(x) Log(x, StackTop())
#else
#    define LOG_STACK_TOP(x)
#endif

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = slice->Code().data();

    const auto* constants = slice->Constants().data();

    constexpr std::array dispatch_table {
        &&op_return,
        &&op_constant,
        &&op_negate,
        &&op_add,
        &&op_sub,
        &&op_div,
        &&op_mul,
    };

#define DISPATCH() goto* dispatch_table[*ip++]

    DISPATCH();

op_return:
    Log("");
    Log("ret {}", Pop());
    return InterpretResult::OK;

op_constant:
    Push(*(constants + *ip++));
    LOG_STACK_TOP("push:  {}");
    DISPATCH();

op_negate:
    *(stack_top - 1) *= -1;
    LOG_STACK_TOP("neg:   {}");
    DISPATCH();

op_add:
    BINARY_OP(+);
    LOG_STACK_TOP("add:   {}");
    DISPATCH();

op_sub:
    BINARY_OP(-);
    LOG_STACK_TOP("sub:   {}");
    DISPATCH();

op_div:
    BINARY_OP(/);
    LOG_STACK_TOP("div:   {}");
    DISPATCH();

op_mul:
    BINARY_OP(*);
    LOG_STACK_TOP("mul:   {}");
    DISPATCH();

    // currently unreachable
    return InterpretResult::RuntimeError;
}

Value VirtualMachine::StackTop() const {
    if (stack_top == &stack.front()) {
        LogErr("Attempted to read from empty stack");
        return 0.0;
    }

    return *(stack_top - 1);
}
}  // namespace hex