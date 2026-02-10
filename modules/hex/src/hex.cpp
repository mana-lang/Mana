#include <hex/hex.hpp>

#include <hex/core/logger.hpp>
#include <hex/core/vm_trace.hpp>

#include <magic_enum/magic_enum.hpp>

#include <array>
#include <print>

namespace hex {
using namespace hexe;

// payloads are little endian
#define READ_PAYLOAD (static_cast<u16>(*ip | *(ip + 1) << 8))
#define NEXT_PAYLOAD (ip += 2, (static_cast<u16>(*(ip - 2) | *(ip - 1) << 8)))

#define CALL_TARGET (static_cast<u32>(*ip | *(ip + 1) << 8 | *(ip + 2) << 16 | *(ip + 3) << 24))

#define REG(idx) registers[frame_offset + (idx)]
#define RETURN_REGISTER registers[REGISTER_RETURN]

/// --- Note ---
/// The reason we don't do bounds checks in Release builds is because they shouldn't be necessary.
///
/// Programs which would send Hex out-of-bounds would be malformed,
/// meaning there is either an internal issue with Hex, or Circe is emitting incorrect bytecode.
/// These issues should never reach users. Users expect speed from Hex.
/// The safety of executing Hexe code is therefore determined by Circe's codegen, and Hex' stability.
/// As Hex' VM loop is relatively simple, we afford ourselves to keep safety checks to Debug builds.
InterpretResult Hex::Execute(ByteCode* bytecode) {
    ip                          = bytecode->EntryPoint();
    auto* const code_start      = bytecode->Instructions().data();
    const auto* const constants = bytecode->Constants().data();

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
        &&call,
        &&print,
        &&print_val,
    };

#ifdef HEX_DEBUG
    constexpr auto dispatch_max = dispatch_table.size();

#   define DISPATCH()                                                                          \
    {                                                                                          \
        TRACE_DISPATCH()                                                                       \
        auto  label = *ip < dispatch_max ? dispatch_table[*ip++] : &&err;                      \
        goto *label;                                                                           \
    }
#else
    // we do no bounds checking whatsoever in release
#   define DISPATCH() goto *dispatch_table[*ip++]
#endif

    // Start VM
    call_stack[++current_function].reg_frame = bytecode->MainRegisterFrame();
    call_stack[current_function].ret_addr    = nullptr; // main doesn't return to anything.
    DISPATCH();

halt:
    std::print("\n\n");
    Log->set_pattern("%^<%n>%$ %v");
    return InterpretResult::OK;

err:
    return InterpretResult::CompileError;

ret: {
        RETURN();
        frame_offset -= call_stack[current_function].reg_frame;
        ip           = call_stack[current_function--].ret_addr;
    }
    DISPATCH();

load_constant: {
        LOADK();
    }
    DISPATCH();

move: {
        MOVE();
    }
    DISPATCH();

add: {
        BINARY_OP(+);
    }
    DISPATCH();

sub: {
        BINARY_OP(-);
    }
    DISPATCH();

div: {
        BINARY_OP(/);
    }
    DISPATCH();

mul: {
        BINARY_OP(*);
    }
    DISPATCH();

mod: {
        BINARY_OP(%);
    }
    DISPATCH();

negate: {
        NEGATE();
    }
    DISPATCH();

bool_not: {
        BOOL_NOT();
    }
    DISPATCH();

cmp_greater: {
        BINARY_OP(>);
    }
    DISPATCH();

cmp_greater_eq: {
        BINARY_OP(>=);
    }
    DISPATCH();

cmp_lesser: {
        BINARY_OP(<);
    }
    DISPATCH();

cmp_lesser_eq: {
        BINARY_OP(<=);
    }
    DISPATCH();

equals: {
        BINARY_OP(==);
    }
    DISPATCH();
not_equals: {
        BINARY_OP(!=);
    }
    DISPATCH();

jmp: {
        JUMP();
    }
    DISPATCH();

jmp_true: {
        JUMP_TRUE();
    }
    DISPATCH();

jmp_false: {
        JUMP_FALSE();
    }
    DISPATCH();

call: {
        // first setup the next stack frame
        frame_offset += *ip;

        call_stack[++current_function].ret_addr = ip + CALL_BYTES;
        call_stack[current_function].reg_frame  = *ip;

        // then call
        ++ip;
        u32 t = CALL_TARGET;
        ip    = code_start + t;
    }
    DISPATCH();

print: {
        const auto s = REG(NEXT_PAYLOAD).AsString();
        std::print("{}", s);
    }
    DISPATCH();

print_val: {
        const auto s = REG(NEXT_PAYLOAD).AsString();
        const auto v = ValueToString(REG(NEXT_PAYLOAD));

        std::vprint_nonunicode(s, std::make_format_args(v));
    }
    DISPATCH();
}

std::string Hex::ValueToString(const Value& v) {
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
    case String:
        return std::string {v.AsString()};
    case None:
        return "none";
    default:
        return "???";
    }
};
} // namespace hex
