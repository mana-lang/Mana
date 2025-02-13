#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
using namespace mana::vm;

InterpretResult VirtualMachine::Interpret(Slice* next_slice) {
    slice = next_slice;
    ip    = slice->Bytecode().data();

    const auto* floats = slice->FloatConstants().data();

    constexpr std::array dispatch_table {
        &&halt,
        &&ret,
        &&push_float,
        &&push_bool,
        &&negate,
        &&add,
        &&sub,
        &&div,
        &&mul,
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
    Log->debug("ret {}\n\n", stack_float.Pop());

    DISPATCH();

push_float:
    stack_float.Push(*(floats + *ip++));
    DISPATCH();

push_bool:
    stack_bool.Push(slice->BoolConstants()[*ip++]);
    DISPATCH();

negate:
    *stack_float.Top() *= -1;
    stack_float.LogTop("neg:   {}");
    DISPATCH();

add:
    stack_float.Op_Add();
    DISPATCH();

sub:
    stack_float.Op_Sub();
    DISPATCH();

div:
    stack_float.Op_Div();
    DISPATCH();

mul:
    stack_float.Op_Mul();
    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

}  // namespace hex