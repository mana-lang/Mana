#include <hex/core/logger.hpp>
#include <hex/vm/virtual-machine.hpp>

namespace hex {
using namespace mana::vm;

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
    Log->debug("ret {}\n\n", stack.Pop().AsFloat());

    DISPATCH();

push:
    stack.Push(*(values + *ip++));
    DISPATCH();

negate:
    stack.Op_Neg();
    DISPATCH();

add:
    stack.Op_Add();
    DISPATCH();

sub:
    stack.Op_Sub();
    DISPATCH();

div:
    stack.Op_Div();
    DISPATCH();

mul:
    stack.Op_Mul();
    DISPATCH();

compile_error:
    return InterpretResult::CompileError;
}

}  // namespace hex