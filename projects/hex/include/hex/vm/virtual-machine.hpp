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
    ml::u8* ip {nullptr};

    std::vector<mvm::Value> locals;
    std::vector<mvm::Value> stack;
    mvm::Value*             stack_top;

public:
    VirtualMachine();

    InterpretResult Interpret(mvm::Slice* next_slice);

    HEX_NODISCARD ml::u64 StackSize() const;

private:
    void Reset();

    void       Push(const mvm::Value& value);
    mvm::Value Pop();

    HEX_NODISCARD mvm::Value ViewTop() const;
    HEX_NODISCARD mvm::Value* StackTop() const;

    void LogTop(std::string_view msg) const;
    void LogTopTwo(std::string_view msg) const;
    void LogLocalDatum(std::string_view msg) const;
};

}  // namespace hex
