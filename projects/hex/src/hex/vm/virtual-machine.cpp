#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <array>

namespace hex {
using namespace mana::vm;

static constexpr std::size_t STACK_MAX = 512;

// @formatter:off

// payloads are little endian
#define READ_PAYLOAD u16((0xFF00 & *ip) | (0x00FF & *(ip+1)))

// jwf inverts the bool output to avoid needing a branch in the vm
// this way falsey multiplies by 1, and truthy multiplies by 0
#define JWF_DIST (READ_PAYLOAD * (((stack_top - 1)->AsBool() - 1) * -1))
#define JWT_DIST (READ_PAYLOAD * (stack_top - 1)->AsBool())

#ifdef HEX_DEBUG
#   define DISPATCH()                                                               \
    {                                                                               \
        auto  label = *ip >= 0 && *ip < dispatch_max ? dispatch_table[*ip++] : err; \
        goto *label;                                                                \
    }
#   define PUSH(val) Push(val)
#   define POP() Pop()
#   define TOP_VAL() *StackTop()
#   define LOG(msg) Log->debug(msg)
#   define LOG_JMP(msg, dist) Log->debug(msg, dist)
#   define LOG_TOP(msg) LogTop(msg)
#   define LOG_TOP_TWO(msg) LogTopTwo(msg)
#   define FETCH_CONSTANT() constants[READ_PAYLOAD]
#   define CMP(op) Pop() op Pop()
#   define LOGICAL_NOT() Push(!Pop())
#else
#   define DISPATCH() goto* dispatch_table[*ip++]
#   define PUSH(val) *(stack_top++) = val
#   define POP() *(--stack_top)
#   define TOP_VAL() *(stack_top - 1)
#   define LOG(msg)
#   define LOG_JMP(msg, dist)
#   define LOG_TOP(msg)
#   define LOG_TOP_TWO(msg)
#   define FETCH_CONSTANT() *(constants + READ_PAYLOAD)
#   define CMP(op) *(stack_top - 2) op *(stack_top - 1)
#   define LOGICAL_NOT() *(++stack_top) =! *(--stack_top)
#endif
// @formatter:on

VirtualMachine::VirtualMachine() {
    stack.resize(STACK_MAX);
    stack_top = stack.data();
}

InterpretResult VirtualMachine::Interpret(Slice* slice) {
    ip = slice->Instructions().data();

    const auto* constants = slice->Constants().data();

    constexpr std::array dispatch_table{
        &&halt,
        &&ret,
        &&push,
        &&pop,
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
        &&jmp,
        &&jwf,
        &&jwt,
    };

#ifdef HEX_DEBUG
    constexpr auto err          = &&compile_error;
    constexpr auto dispatch_max = dispatch_table.size();
#endif

    constexpr auto payload_size = 2;

    // Start VM
    DISPATCH();

halt:
    return InterpretResult::OK;

ret:
#ifdef HEX_DEBUG
    Log->debug("");

    if (StackTop()->GetType() == mana::PrimitiveType::Bool) {
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
    ip += payload_size;
    LOG_TOP("[push:  {}]");

    DISPATCH();

pop:
    LOG_TOP("[pop:  {}]");
    POP();

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

jmp:
    LOG_JMP("jmp: {}", READ_PAYLOAD);
    ip += READ_PAYLOAD + payload_size;

    DISPATCH();

jwf:
    LOG_JMP("jmp-false: {}", JWF_DIST);
    ip += JWF_DIST + payload_size;

    DISPATCH();

jwt:
    LOG_JMP("jmp-true: {}", JWT_DIST);
    ip += JWT_DIST + payload_size;

    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

void VirtualMachine::Reset() {
    stack_top = stack.data();
}

void VirtualMachine::Push(const Value& value) {
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
    if (StackTop()->GetType() == mana::PrimitiveType::Bool) {
        Log->debug(fmt::runtime(msg), ViewTop().AsBool());
    } else {
        Log->debug(fmt::runtime(msg), ViewTop().AsFloat());
    }
}

void VirtualMachine::LogTopTwo(const std::string_view msg) const {
    if (StackTop()->GetType() == mana::PrimitiveType::Bool) {
        Log->debug(fmt::runtime(msg), (StackTop() - 1)->AsBool(), ViewTop().AsBool());
    } else {
        Log->debug(fmt::runtime(msg), (StackTop() - 1)->AsFloat(), ViewTop().AsFloat());
    }
}
} // namespace hex
