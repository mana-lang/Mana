#pragma once

// this file contains macros for printing out certain Hex instructions mid-execution
// since Mana has a Print builtin, these should generally be avoided, as it
// drastically slows down execution. But it may help with debugging

// return
#ifdef HEX_DEBUG
#   define RETURN()                      \
        const auto src  = NEXT_PAYLOAD;  \
        RETURN_REGISTER = REG(src);      \
        Log->debug("  <- R{} ({})", src + frame_offset, ValueToString(RETURN_REGISTER));
#else
#   define RETURN() RETURN_REGISTER = REG(NEXT_PAYLOAD);
#endif


// loadk
#ifdef HEX_DEBUG
#   define LOADK()                 \
        u16 dst  = NEXT_PAYLOAD;   \
        u16 idx  = NEXT_PAYLOAD;   \
        REG(dst) = constants[idx]; \
        Log->debug("  R{} <- {} (const #{})", dst + frame_offset, ValueToString(REG(dst)), idx);
#else
#   define LOADK()                 \
        u16 dst  = NEXT_PAYLOAD;   \
        REG(dst) = constants[NEXT_PAYLOAD];
#endif


// move
#ifdef HEX_DEBUG
#   define MOVE()                 \
        u16 dst  = NEXT_PAYLOAD;  \
        u16 src  = NEXT_PAYLOAD;  \
        REG(dst) = REG(src);      \
        Log->debug("  R{} <- R{} ({})", dst + frame_offset, src + frame_offset, ValueToString(REG(dst)));
#else
#   define MOVE()                 \
        u16 dst  = NEXT_PAYLOAD;  \
        REG(dst) = REG(NEXT_PAYLOAD);
#endif


// binary_op
#ifdef HEX_DEBUG
#   define BINARY_OP(op)                                      \
        {                                                     \
        u16 dst = NEXT_PAYLOAD;                               \
        u16 lhs = NEXT_PAYLOAD;                               \
        u16 rhs = NEXT_PAYLOAD;                               \
        std::string lhs_orig = ValueToString(REG(lhs));       \
        REG(dst) = REG(lhs) op REG(rhs);                      \
        Log->debug("  R{} ({}) = R{} ({}) {} R{} ({})",       \
               dst + frame_offset, ValueToString(REG(dst)),   \
               lhs + frame_offset, lhs_orig,                  \
               #op,                                           \
               rhs + frame_offset, ValueToString(REG(rhs)));  \
       }
#else
#   define BINARY_OP(op)        \
        u16 dst = NEXT_PAYLOAD; \
        u16 lhs = NEXT_PAYLOAD; \
        u16 rhs = NEXT_PAYLOAD; \
        REG(dst) = REG(lhs) op REG(rhs)
#endif


// negate
#ifdef HEX_DEBUG
#   define NEGATE()                          \
        u16 dst  = NEXT_PAYLOAD;             \
        u16 src  = NEXT_PAYLOAD;             \
        REG(dst) = -REG(src);                \
        Log->debug("  R{} ({}) = -R{} ({})", \
                   dst + frame_offset,       \
                   ValueToString(REG(dst)),  \
                   src + frame_offset,       \
                   ValueToString(REG(src))   \
        );
#else
#   define NEGATE()               \
        u16 dst  = NEXT_PAYLOAD;  \
        REG(dst) = -REG(NEXT_PAYLOAD);
#endif


// bool_not
#ifdef HEX_DEBUG
#   define BOOL_NOT()                         \
        u16 dst  = NEXT_PAYLOAD;              \
        u16 src  = NEXT_PAYLOAD;              \
        REG(dst) = !REG(src);                 \
        Log->debug("  R{} ({}) = !R{} ({})",  \
                   dst + frame_offset,        \
                   ValueToString(REG(dst)),   \
                   src + frame_offset,        \
                   ValueToString(REG(src))    \
        );
#else
#   define BOOL_NOT()             \
        u16 dst  = NEXT_PAYLOAD;  \
        REG(dst) = !REG(NEXT_PAYLOAD);
#endif


// jump
#ifdef HEX_DEBUG
// jumps are stored as u16, but encoded as i16
// so we need to convert them back here
#   define JUMP()                                                         \
        i16 dist = static_cast<i16>(NEXT_PAYLOAD);                        \
        const auto target = ip - bytecode->Instructions().data() + dist;  \
        Log->debug("  Jump ==> [{:04}]", target);                         \
        ip += dist;
#else
#   define JUMP() ip += static_cast<i16>(NEXT_PAYLOAD);
#endif


// jump_true
#ifdef HEX_DEBUG
#   define JUMP_TRUE()                                                      \
        const u16 target    = NEXT_PAYLOAD;                                 \
        const auto dist  = static_cast<i16>(NEXT_PAYLOAD);                  \
        const auto taken = REG(target).AsBool();                            \
        const auto addr  = ip - bytecode->Instructions().data() + dist;     \
        Log->debug("  Jump ==> [{:04}] R{} ({}) => {}",                     \
                   addr,                                                    \
                   target + frame_offset,                                   \
                   ValueToString(REG(target)),                              \
                   taken ? "TAKEN" : "SKIP"                                 \
        );                                                                  \
        ip += dist * REG(target).AsBool();
#else
// sidestep branch predictions altogether
#   define JUMP_TRUE()                       \
        const u16 target  = NEXT_PAYLOAD;    \
        ip += static_cast<i16>(NEXT_PAYLOAD) * REG(target).AsBool();
#endif


// jump_false
#ifdef HEX_DEBUG
// this just inverts the output
#   define JUMP_FALSE()                                                 \
        u16 target       = NEXT_PAYLOAD;                                \
        i16 dist         = static_cast<i16>(NEXT_PAYLOAD);              \
        const bool taken = !REG(target).AsBool();                       \
        const auto addr  = ip - bytecode->Instructions().data() + dist; \
        Log->debug("  Jump ==> [{:04}] R{} ({}) => {}",                 \
                   addr,                                                \
                   target + frame_offset,                               \
                   ValueToString(REG(target)),                          \
                   taken ? "TAKEN" : "SKIPPED"                          \
        );                                                              \
        ip += dist * ((REG(target).AsBool() - 1) * -1);
#else
#   define JUMP_FALSE()                    \
        u16 target = NEXT_PAYLOAD;         \
        ip += static_cast<i16>(NEXT_PAYLOAD) * ((REG(target).AsBool() - 1) * -1);
#endif
