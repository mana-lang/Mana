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
#   define DISPATCH()                                                                   \
        {                                                                               \
            auto  label = *ip >= 0 && *ip < dispatch_max ? dispatch_table[*ip++] : err; \
            goto *label;                                                                \
        }
#   define PUSH(val) Push(val)
#   define POP() Pop()
#   define TOP_VAL() *StackTop()
#   define LOG(msg) Log->debug(msg)
#   define LOG_TOP(msg) LogTop(msg)
#   define LOG_TOP_TWO(msg) LogTopTwo(msg)
#   define FETCH_CONSTANT() values[*ip++]
#   define CMP(op) Pop() op Pop()
#   define LOGICAL_NOT() Push(!Pop())
#else
#   define DISPATCH() goto* dispatch_table[*ip++]
#   define PUSH(val) *(stack_top++) = val
#   define POP() *(--stack_top)
#   define TOP_VAL() *(stack_top - 1)
#   define LOG(msg)
#   define LOG_TOP(msg)
#   define LOG_TOP_TWO(msg)
#   define FETCH_CONSTANT() *(values + *ip++)
#   define CMP(op) *(stack_top - 2) op *(stack_top - 1)
#   define LOGICAL_NOT() *(++stack_top) = !*(--stack_top)
#endif
    // clang-format on


    // Start VM
    DISPATCH();

halt:
    return InterpretResult::OK;

ret:
#ifdef HEX_DEBUG
    Log->debug("");

    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug("ret {}\n\n", Pop().AsBool());
    } else {
        Log->debug("ret {}\n\n", Pop().AsFloat());
    }
#else
    POP();
#endif

    DISPATCH();

push:
    PUSH(FETCH_CONSTANT());
    LOG_TOP("[push:  {}]");

    DISPATCH();

negate:
    TOP_VAL() *= -1;
    LOG_TOP("neg:   {}");

    DISPATCH();

add:
    LOG_TOP_TWO("add: ({}, {})");
    TOP_VAL() += POP();

    DISPATCH();

sub:
    LOG_TOP_TWO("sub: ({}, {})");
    TOP_VAL() -= POP();

    DISPATCH();

div:
    LOG_TOP_TWO("div: ({}, {})");
    TOP_VAL() /= POP();

    DISPATCH();

mul:
    LOG_TOP_TWO("mul: ({}, {})");
    TOP_VAL() *= POP();

    DISPATCH();

cmp_greater:
    LOG_TOP_TWO("cmp_greater: ({}, {})");
    PUSH(CMP(>));

    DISPATCH();

cmp_greater_eq:
    LOG_TOP_TWO("cmp_greater_eq: ({}, {})");
    PUSH(CMP(>=));

    DISPATCH();

cmp_lesser:
    LOG_TOP_TWO("cmp_lesser: ({}, {})");
    PUSH(CMP(<));

    DISPATCH();

cmp_lesser_eq:
    LOG_TOP_TWO("cmp_lesser_eq: ({}, {})");
    PUSH(CMP(<=));

    DISPATCH();

equals:
    LOG_TOP_TWO("equals ({}, {})");
    PUSH(CMP(==));

    DISPATCH();

not_equals:
    LOG_TOP_TWO("not_equals ({}, {})");
    PUSH(CMP(!=));

    DISPATCH();

bool_not:
    LOG_TOP("not ({})");
    LOGICAL_NOT();

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
    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug(fmt::runtime(msg), ViewTop().AsBool());
    } else {
        Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
    }
}

void VirtualMachine::LogTopTwo(std::string_view msg) const {
    if (StackTop()->GetType() == Value::Type::Bool) {
        Log->debug(fmt::runtime(msg), (StackTop() - 1)->AsBool(), ViewTop().AsBool());
    } else {
        Log->debug(fmt::runtime(msg), (StackTop() - 1)->AsFloat(), ViewTop().AsFloat());
    }
}

}  // namespace hex