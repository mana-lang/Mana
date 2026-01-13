#pragma once

#include <sigil/ast/token.hpp>

#include <vector>
#include <string>
#include <string_view>

using TokenStream = std::vector<sigil::Token>;

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

// this doesn't need to be all that performant
template <StringLike T, StringLike... Rest>
auto Concatenate(T first, Rest... rest) {
    return std::string(first) + std::string(rest...);
}

inline auto StripRedundant(const TokenStream& base_tokens) -> TokenStream {
    auto tokens = decltype(base_tokens) {};
    tokens.reserve(base_tokens.size());

    for (const auto& token : base_tokens) {
        using enum sigil::TokenType;
        if (token.type == Terminator) {
            continue;
        }
        tokens.push_back(token);
    }

    return tokens;
}
