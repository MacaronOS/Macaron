#pragma once

#include "Common.hpp"
#include "HashFunctions.hpp"
#include "Runtime.hpp"

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

template <typename T>
requires(IsIntegral<T> || IsPointer<T>) struct Traits<T> : public GenericTraits<T> {
    static constexpr uint32_t hash(T value)
    {
        if constexpr (sizeof(T) < 8) {
            return u32_hash((uint32_t)value);
        } else {
            return u64_hash((uint64_t)value);
        }
    }
};
