#pragma once

#include <salem/frontend/token.hpp>

template<class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template<StringLike String, StringLike... Rest>
auto concatenate(String first, Rest... rest) {
    return std::string(first) + std::string(rest...);
}

inline auto strip_redundant(const salem::token_stream& base_tokens) -> salem::token_stream {
    auto tokens = decltype(base_tokens){};
    tokens.reserve(base_tokens.size());

    for (const auto& t : base_tokens) {
        using enum salem::token_type;
        if (t.type_ == Terminator || t.type_ == _module_) {
            continue;
        }
        tokens.push_back(t);
    }
    return tokens;
}