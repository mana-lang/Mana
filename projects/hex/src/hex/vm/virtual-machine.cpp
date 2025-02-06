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

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = &slice->Code()[0];

    return Run();
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

InterpretResult VirtualMachine::Run() {
    while (true) {
        switch (static_cast<Op>(*ip++)) {
        case Op::Constant: {
            Push(slice->ConstantAt(*ip++));
            LOG_STACK_TOP("push:  {}");
            break;
        }
        case Op::Negate:
            Push(-Pop());
            LOG_STACK_TOP("neg:   {}");
            break;
        case Op::Add:
            BIN_OP(+);
            LOG_STACK_TOP("add:   {}");
            break;
        case Op::Sub:
            BIN_OP(-);
            LOG_STACK_TOP("sub:   {}");
            break;
        case Op::Div:
            BIN_OP(/);
            LOG_STACK_TOP("div:   {}");
            break;
        case Op::Mul:
            BIN_OP(*);
            LOG_STACK_TOP("mul:   {}");
            break;
        case Op::Return:
            Log("");
            Log("ret {}", Pop());
            return InterpretResult::OK;
        default:
            return InterpretResult::RuntimeError;
        }
    }
}

Value VirtualMachine::StackTop() const {
    if (stack_top == &stack[0]) {
        LogErr("StackTop could not be printed as stack is empty.");
        return 0.0;
    }

    return *(stack_top - 1);
}
}  // namespace hex