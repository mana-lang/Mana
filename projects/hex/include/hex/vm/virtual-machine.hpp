#pragma once

#include <mana/literals.hpp>
#include <mana/vm/slice.hpp>

namespace hex {
namespace ml  = mana::literals;
namespace mvm = mana::vm;

enum class InterpretResult {
    OK,
    CompileError,
    RuntimeError,
};

class VirtualMachine {
    mvm::Slice* slice {nullptr};
    ml::u8*     ip {nullptr};

    // Stack stack {};
    std::vector<mvm::Value> stack;
    mvm::Value*             stack_top;

public:
    VirtualMachine();

    InterpretResult Interpret(mvm::Slice* next_slice);

private:
    void Reset();
    void Push(mvm::Value value);

    mvm::Value  Pop();
    mvm::Value  ViewTop() const;
    mvm::Value* StackTop() const;

    void LogTop(std::string_view msg) const;
    void LogTopBool(std::string_view msg) const;
};

}  // namespace hex
