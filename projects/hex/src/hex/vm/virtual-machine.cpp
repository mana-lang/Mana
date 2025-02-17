#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
using namespace mana::vm;

VirtualMachine::VirtualMachine() {
    stack.reserve(128);
    stack_top = stack.data();
}

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = slice->Instructions().data();

    const auto* values = slice->Constants().data();

    constexpr std::array dispatch_table {
        &&halt,
        &&ret,
        &&push,
        &&negate,
        &&add,
        &&sub,
        &&div,
        &&mul,
        &&cmp_greater,
        &&cmp_lesser,
    };

    // clang-format off
#ifdef HEX_DEBUG
    constexpr auto err          = &&compile_error;
    constexpr auto dispatch_max = dispatch_table.size();
#    define DISPATCH()                                                                  \
        {                                                                               \
            auto  label = *ip >= 0 && *ip < dispatch_max ? dispatch_table[*ip++] : err; \
            goto *label;                                                                \
        }
#else
#    define DISPATCH() goto* dispatch_table[*ip++]
#endif
    // clang-format on

    DISPATCH();

halt:
    return InterpretResult::OK;

ret:
    Log->debug("");

    if (StackTop()->GetType() == Value::Type::Boolean) {
        Log->debug("ret {}\n\n", Pop().AsBool());
    } else {
        Log->debug("ret {}\n\n", Pop().AsFloat());
    }

    DISPATCH();

push:
    Push(*(values + *ip++));
    DISPATCH();

negate:
    *(stack_top - 1) *= -1;

    LogTop("neg:   {}");
    DISPATCH();

add:
    *(stack_top - 1) += Pop();

    LogTop("add:   {}");
    DISPATCH();

sub:
    *(stack_top - 1) -= Pop();

    LogTop("sub:   {}");
    DISPATCH();

div:
    *(stack_top - 1) /= Pop();

    LogTop("div:   {}");
    DISPATCH();

mul:
    *(stack_top - 1) *= Pop();

    LogTop("mul:   {}");
    DISPATCH();

cmp_greater:
    Push(Pop() > Pop());

    LogTopBool("cmp_greater: {}");
    DISPATCH();

cmp_lesser:
    Push(Pop() < Pop());

    LogTopBool("cmp_lesser: {}");
    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

void VirtualMachine::Reset() {
    stack_top = stack.data();
}

void VirtualMachine::Push(Value value) {
    if (stack_top == &stack.back()) {
        stack.reserve(stack.capacity() * 2);
    }

    *stack_top = value;
    ++stack_top;

    LogTop("push:  {}");
}

Value VirtualMachine::Pop() {
    if (stack_top != &stack.front()) {
        --stack_top;
    } else {
        Log->error("Attempted to pop from empty stack.");
        return 0.0;
    }

    return *stack_top;
}

Value VirtualMachine::ViewTop() const {
    if (stack_top == &stack.front()) {
        Log->error("Attempted to read from empty stack");
        return 0.0;
    }

    return *(stack_top - 1);
}

Value* VirtualMachine::StackTop() const {
    if (stack_top == &stack.front()) {
        Log->error("Attempted to read from empty stack");
        return nullptr;
    }

    return stack_top - 1;
}

void VirtualMachine::LogTop(const std::string_view msg) const {
#ifdef HEX_DEBUG
    Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
#endif
}

void VirtualMachine::LogTopBool(const std::string_view msg) const {
#ifdef HEX_DEBUG
    Log->debug(fmt::runtime(msg), ViewTop().AsBool());
#endif
}

}  // namespace hex