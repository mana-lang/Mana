#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
using namespace mana::vm;

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = slice->Code().data();

    const auto* constants = slice->Constants().data();

    constexpr std::array dispatch_table {
        &&op_halt,
        &&op_return,
        &&op_constant,
        &&op_negate,
        &&op_add,
        &&op_sub,
        &&op_div,
        &&op_mul,
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

op_halt:
    return InterpretResult::OK;

op_return:
    Log->debug("");
    Log->debug("ret {}\n\n", stack_f64.Pop());

    DISPATCH();

op_constant:
    stack_f64.Push(*(constants + *ip++));
    DISPATCH();

op_negate:
    *stack_f64.Top() *= -1;
    stack_f64.LogTop("neg:   {}");
    DISPATCH();

op_add:
    stack_f64.Op_Add();
    DISPATCH();

op_sub:
    stack_f64.Op_Sub();
    DISPATCH();

op_div:
    stack_f64.Op_Div();
    DISPATCH();

op_mul:
    stack_f64.Op_Mul();
    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

}  // namespace hex