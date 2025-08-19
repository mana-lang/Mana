#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
using namespace mana::vm;

static constexpr std::size_t STACK_MAX = 512;

VirtualMachine::VirtualMachine() {
    stack.resize(STACK_MAX);
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
        &&cmp_greater_eq,
        &&cmp_lesser,
        &&cmp_lesser_eq,
        &&equals,
        &&not_equals,
        &&bool_not,
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

    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug("ret {}\n\n", Pop().AsBool());
    } else {
        Log->debug("ret {}\n\n", Pop().AsFloat());
    }

    DISPATCH();

push:
    Push(*(values + *ip++));

    LogTop("[push:  {}]");
    DISPATCH();

negate:
    *(stack_top - 1) *= -1;

    LogTop("neg:   {}");
    DISPATCH();

add:
    LogTopTwo("add: ({}, {})");

    *(stack_top - 1) += Pop();
    DISPATCH();

sub:
    LogTopTwo("sub: ({}, {})");

    *(stack_top - 1) -= Pop();
    DISPATCH();

div:
    LogTopTwo("div: ({}, {})");

    *(stack_top - 1) /= Pop();
    DISPATCH();

mul:
    LogTopTwo("mul: ({}, {})");

    *(stack_top - 1) *= Pop();
    DISPATCH();

cmp_greater:
    LogTopTwo("cmp_greater: ({}, {})");

    Push(Pop() > Pop());
    DISPATCH();

cmp_greater_eq:
    LogTopTwo("cmp_greater_eq: ({}, {})");

    Push(Pop() >= Pop());
    DISPATCH();

cmp_lesser:
    LogTop("cmp_lesser: ({}, {})");

    Push(Pop() < Pop());
    DISPATCH();

cmp_lesser_eq:
    LogTopTwo("cmp_lesser_eq: ({}, {})");

    Push(Pop() <= Pop());
    DISPATCH();

equals:
    LogTopTwo("equals ({}, {})");

    Push(Pop() == Pop());
    DISPATCH();

not_equals:
    LogTopTwo("not_equals ({}, {})");

    Push(Pop() != Pop());
    DISPATCH();

bool_not:
    LogTop("not ({})");

    Push(!Pop());
    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

void VirtualMachine::Reset() {
    stack_top = stack.data();
}

void VirtualMachine::Push(const Value value) {
    if (stack_top == &stack.back()) {
        stack.resize(stack.capacity() * 2);
    }

    *stack_top = value;
    ++stack_top;
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
    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug(fmt::runtime(msg), ViewTop().AsBool());
    } else {
        Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
    }
#endif
}

void VirtualMachine::LogTopTwo(std::string_view msg) const {
#ifdef HEX_DEBUG
    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug(fmt::runtime(msg), ViewTop().AsBool(), (StackTop() - 1)->AsBool());
    } else {
        Log->debug(fmt::runtime(msg), ViewTop().AsFloat(), (StackTop() - 1)->AsFloat());
    }
#endif
}

}  // namespace hex