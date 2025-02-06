#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
VirtualMachine::VirtualMachine()
    : stack_top(&stack[0]) {}

void VirtualMachine::ResetStack() {
    stack_top = &stack[0];
}

void VirtualMachine::Push(const Value value) {
    *stack_top = value;
    ++stack_top;
}

Value VirtualMachine::Pop() {
    --stack_top;
    return *stack_top;
}

#define BIN_OP(x)        \
    {                    \
        Value b = Pop(); \
        Value a = Pop(); \
        Push(a x b);     \
    }

#ifdef HEX_DEBUG
#    define LOG_STACK_TOP(x) Log(x, StackTop())
#else
#    define LOG_STACK_TOP(x)
#endif

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = &slice->Code()[0];

    static const std::array dispatch_table {
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
    Push(slice->ConstantAt(*ip++));
    LOG_STACK_TOP("push:  {}");
    DISPATCH();

op_negate:
    Push(-Pop());
    LOG_STACK_TOP("neg:   {}");
    DISPATCH();

op_add:
    BIN_OP(+);
    LOG_STACK_TOP("add:   {}");
    DISPATCH();

op_sub:
    BIN_OP(-);
    LOG_STACK_TOP("sub:   {}");
    DISPATCH();

op_div:
    BIN_OP(/);
    LOG_STACK_TOP("div:   {}");
    DISPATCH();

op_mul:
    BIN_OP(*);
    LOG_STACK_TOP("mul:   {}");
    DISPATCH();

    // currently unreachable
    return InterpretResult::RuntimeError;
}

Value VirtualMachine::StackTop() const {
    if (stack_top == &stack[0]) {
        LogErr("StackTop could not be printed as stack is empty.");
        return 0.0;
    }

    return *(stack_top - 1);
}
}  // namespace hex