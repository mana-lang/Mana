#pragma once

#include <hex/ast/token.hpp>

template<class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

// this doesn't need to be all that performant
template<StringLike T, StringLike... Rest>
auto Concatenate(T first, Rest ... rest) {
  return std::string(first) + std::string(rest...);
}

inline auto StripRedundant(const hex::TokenStream& base_tokens) -> hex::TokenStream {
  auto tokens = decltype(base_tokens){};
  tokens.reserve(base_tokens.size());

  for (const auto& token : base_tokens) {
    using enum hex::TokenType;
    if (token.type == Terminator || token.type == _module_) {
      continue;
    }
    tokens.push_back(token);
  }

  return tokens;
}