#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

#include <magic_enum/magic_enum.hpp>

#include <array>

namespace hex {
using namespace mana::vm;

static constexpr std::size_t BASE_REG_SIZE = 256;

// payloads are little endian
#define READ_PAYLOAD (static_cast<u16>(*ip | *(ip + 1) << 8))

#define NEXT_PAYLOAD (ip += 2, (static_cast<u16>(*(ip - 2) | *(ip - 1) << 8)))
#define REG(idx) registers[idx]

VirtualMachine::VirtualMachine() {
    registers.resize(BASE_REG_SIZE);
}

InterpretResult VirtualMachine::Interpret(Hexe* slice) {
    ip = slice->Instructions().data();

    const auto* constants = slice->Constants().data();

    // this is for computed goto
    // it's important to note this list's order is rigid
    // it must /exactly/ match the opcode enum order
    constexpr std::array dispatch_table {
        &&halt,
        &&err,
        &&ret,
        &&load_constant,
        &&move,
        &&add,
        &&sub,
        &&div,
        &&mul,
        &&mod,
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
    const auto ValueToString = [](const Value& v) -> std::string {
        using namespace mana;
        switch (v.GetType()) {
        case Int64:
            return std::to_string(v.AsInt());
        case Uint64:
            return std::to_string(v.AsUint());
        case Float64:
            return fmt::format("{:.2f}", v.AsFloat());
        case Bool:
            return v.AsBool() ? "true" : "false";
        case None:
            return "none";
        default:
            return "???";
        }
    };
#   define DISPATCH()                                                   \
    {                                                                   \
        const auto offset = ip - slice->Instructions().data();                                 \
        if (offset < slice->Instructions().size()) {                                           \
            Log->debug("{:04} | {:<16}", offset, magic_enum::enum_name(static_cast<Op>(*ip))); \
        }                                                                                      \
        auto  label = *ip < dispatch_max ? dispatch_table[*ip++] : &&compile_error;  \
        goto *label;                                                                 \
    }

    constexpr auto dispatch_max = dispatch_table.size();
#else
    // we do no bounds checking whatsoever in release
#   define DISPATCH() goto *dispatch_table[*ip++]
#endif


    // Start VM
    DISPATCH();

halt:
    return InterpretResult::OK;

err:
    return InterpretResult::CompileError;

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

#ifdef HEX_DEBUG
        Log->debug("  R{} <- {} (const #{})", dst, ValueToString(REG(dst)), idx);
#endif
        DISPATCH();
    }
move: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = REG(src);

#ifdef HEX_DEBUG
        Log->debug("  R{} <- R{} ({})", dst, src, ValueToString(REG(dst)));
#endif

        DISPATCH();
    }

#ifdef HEX_DEBUG
#define BINARY_OP(op)                                 \
    {                                                 \
    u16 dst = NEXT_PAYLOAD;                           \
    u16 lhs = NEXT_PAYLOAD;                           \
    u16 rhs = NEXT_PAYLOAD;                           \
    std::string lhs_orig = ValueToString(REG(lhs));   \
    REG(dst) = REG(lhs) op REG(rhs);                  \
    Log->debug("  R{} ({}) = R{} ({}) {} R{} ({})",   \
               dst, ValueToString(REG(dst)),          \
               lhs, lhs_orig,                         \
               #op,                                   \
               rhs, ValueToString(REG(rhs)));         \
    }
#else
#define BINARY_OP(op)       \
    u16 dst = NEXT_PAYLOAD; \
    u16 lhs = NEXT_PAYLOAD; \
    u16 rhs = NEXT_PAYLOAD; \
    REG(dst) = REG(lhs) op REG(rhs)
#endif

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

mod: {
        BINARY_OP(%);
        DISPATCH();
    }

negate: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = -REG(src);

#ifdef HEX_DEBUG
        Log->debug("  R{} ({}) = -R{} ({})", dst, ValueToString(REG(dst)), src, ValueToString(REG(src)));
#endif

        DISPATCH();
    }

bool_not: {
        u16 dst  = NEXT_PAYLOAD;
        u16 src  = NEXT_PAYLOAD;
        REG(dst) = !REG(src);

#ifdef HEX_DEBUG
        Log->debug("  R{} ({}) = !R{} ({})", dst, ValueToString(REG(dst)), src, ValueToString(REG(src)));
#endif

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
        // jumps are stored as u16, but encoded as i16
        // so we need to convert them back here
#ifdef HEX_DEBUG
        i16 dist = static_cast<i16>(NEXT_PAYLOAD);

        const auto target = ip - slice->Instructions().data() + dist;
        Log->debug("  Jump ==> [{:04}]", target);
        ip += dist;
#else
        ip += static_cast<i16>(NEXT_PAYLOAD);
#endif

        DISPATCH();
    }

jmp_true: {
        u16 reg  = NEXT_PAYLOAD;
        i16 dist = static_cast<i16>(NEXT_PAYLOAD);

#ifdef HEX_DEBUG
        const bool taken  = REG(reg).AsBool();
        const auto target = ip - slice->Instructions().data() + dist;
        Log->debug("  Jump ==> [{:04}] R{} ({}) => {}",
                   target,
                   reg,
                   ValueToString(REG(reg)),
                   taken ? "TAKEN" : "SKIP"
        );
#endif

        // sidestep branch predictions altogether
        ip += dist * REG(reg).AsBool();


        DISPATCH();
    }
jmp_false: {
        u16 reg  = NEXT_PAYLOAD;
        i16 dist = static_cast<i16>(NEXT_PAYLOAD);

#ifdef HEX_DEBUG
        const bool taken  = !REG(reg).AsBool();
        const auto target = ip - slice->Instructions().data() + dist;
        Log->debug("  Jump ==> [{:04}] R{} ({}) => {}",
                   target,
                   reg,
                   ValueToString(REG(reg)),
                   taken ? "TAKEN" : "SKIPPED"
        );
#endif

        // this just inverts the output
        ip += dist * ((REG(reg).AsBool() - 1) * -1);

        DISPATCH();
    }

compile_error: {
        return InterpretResult::CompileError;
    }
}
} // namespace hex
