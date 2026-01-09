#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <array>

namespace hex {
using namespace mana::vm;

static constexpr std::size_t BASE_REG_SIZE = 32;

// payloads are little endian
#define READ_PAYLOAD (static_cast<u16>(*ip | *(ip + 1) << 8))

#define NEXT_PAYLOAD (ip += 2, (static_cast<u16>(*(ip - 2) | *(ip - 1) << 8)))
#define REG(idx) registers[idx]

VirtualMachine::VirtualMachine() {
    registers.resize(BASE_REG_SIZE);
}

InterpretResult VirtualMachine::Interpret(Slice* slice) {
    ip = slice->Instructions().data();

    const auto* constants = slice->Constants().data();

    // this is for computed goto
    // it's important to note this list's order is rigid
    // it must /exactly/ match the opcode enum order
    constexpr std::array dispatch_table {
        &&halt,
        &&ret,
        &&load_constant,
        &&move,
        &&add,
        &&sub,
        &&div,
        &&mul,
        &&negate,
        &&bool_not,
        &&cmp_greater,
        &&cmp_greater_eq,
        &&cmp_lesser,
        &&cmp_lesser_eq,
        &&equals,
        &&not_equals,
        &&jmp,
        &&jmp_true,
        &&jmp_false,
    };

#ifdef HEX_DEBUG
#   define DISPATCH()                                                   \
    {                                                                   \
        auto  label = *ip < dispatch_max ? dispatch_table[*ip++] : err; \
        goto *label;                                                    \
    }

    constexpr auto err          = &&compile_error;
    constexpr auto dispatch_max = dispatch_table.size();
#else
    // we do no bounds checking whatsoever in release
#   define DISPATCH() goto *dispatch_table[*ip++]
#endif


    // Start VM
    DISPATCH();

halt:
    return InterpretResult::OK;

ret: {
        u16 reg = NEXT_PAYLOAD;
#ifdef HEX_DEBUG
        Log->debug("");
        Log->debug("[ret: {} <- R{}]", REG(reg).AsFloat(), reg);
#endif

        DISPATCH();
    }

load_constant: {
        u16 dst  = NEXT_PAYLOAD;
        u16 idx  = NEXT_PAYLOAD;
        REG(dst) = constants[idx];

        DISPATCH();
    }
move: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = REG(src);

        DISPATCH();
    }

#define BINARY_OP(op)       \
    u16 dst = NEXT_PAYLOAD; \
    u16 lhs = NEXT_PAYLOAD; \
    u16 rhs = NEXT_PAYLOAD; \
    REG(dst) = REG(lhs) op REG(rhs)

add: {
        BINARY_OP(+);
        DISPATCH();
    }

sub: {
        BINARY_OP(-);
        DISPATCH();
    }

div: {
        BINARY_OP(/);
        DISPATCH();
    }

mul: {
        BINARY_OP(*);
        DISPATCH();
    }

negate: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = -REG(src);

        DISPATCH();
    }

bool_not: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = !REG(src);

        DISPATCH();
    }

cmp_greater: {
        BINARY_OP(>);
        DISPATCH();
    }

cmp_greater_eq: {
        BINARY_OP(>=);
        DISPATCH();
    }

cmp_lesser: {
        BINARY_OP(<);
        DISPATCH();
    }

cmp_lesser_eq: {
        BINARY_OP(<=);
        DISPATCH();
    }

equals: {
        BINARY_OP(==);
        DISPATCH();
    }

not_equals: {
        BINARY_OP(!=);
        DISPATCH();
    }

jmp: {
        ip += NEXT_PAYLOAD;
        DISPATCH();
    }

jmp_true: {
        u16 reg  = NEXT_PAYLOAD;
        u16 dist = NEXT_PAYLOAD;

        // sidestep branch predictions altogether
        ip += dist * REG(reg).AsBool();

        DISPATCH();
    }
jmp_false: {
        u16 reg  = NEXT_PAYLOAD;
        u16 dist = NEXT_PAYLOAD;

        // this just inverts the output
        ip += dist * ((REG(reg).AsBool() - 1) * -1);

        DISPATCH();
    }

compile_error: {
        return InterpretResult::CompileError;
    }
}
} // namespace hex
