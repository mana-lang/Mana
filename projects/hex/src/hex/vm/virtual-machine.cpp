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

InterpretResult VirtualMachine::Run() {
    while (true) {
        switch (static_cast<Op>(*ip++)) {
        case Op::Constant: {
            Value constant = slice->ConstantAt(*ip++);
            Log("push | {}", constant);
            Push(constant);
            break;
        }
        case Op::Negate: {
            const Value c = -Pop();
            Push(c);
            ip++;
            Log("neg  | {}", c);
            break;
        }
        case Op::Return:
            Log("{}", Pop());
            return InterpretResult::OK;
        default:
            return InterpretResult::RuntimeError;
        }
    }
}

void VirtualMachine::BinOp() {
    Value b = Pop();
    Value a = Pop();
    Push()
}
}  // namespace hex