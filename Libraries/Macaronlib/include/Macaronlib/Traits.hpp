#pragma once

template <typename T>
struct GenericTraits {
    using ArgType = const T&;
    using Reference = T&;
    using ConstReference = const T&;

    static constexpr bool equals(const T& a, const T& b) { return a == b; }
};

template <typename T>
struct Traits : public GenericTraits<T> {
};

template <>
struct Traits<char> : public GenericTraits<char> {
    using ArgType = char;
};
