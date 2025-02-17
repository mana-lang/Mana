#pragma once

#include <hex/core/logger.hpp>

#include <mana/vm/value.hpp>
#include <string_view>
#include <vector>

namespace hex {
namespace mvm = mana::vm;

class Stack {
    std::vector<mvm::Value> values {};
    mvm::Value*             top {nullptr};

public:
    Stack();

    void Reset();
    void Push(mvm::Value value);

    mvm::Value  Pop();
    mvm::Value  ViewTop() const;
    mvm::Value* Top() const;

    void LogTop(std::string_view msg) const;
    void LogTopBool(std::string_view msg) const;

    void Op_Add();
    void Op_Sub();
    void Op_Mul();
    void Op_Div();
    void Op_Neg();

    void Op_CmpGreater();
    void Op_CmpLesser();
};
}  // namespace hex