#pragma once
#include <type_traits>

template <typename T>
concept LiteralType = std::is_arithmetic_v<T> || std::is_same_v<T, void>;
