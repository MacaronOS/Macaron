#pragma once

#include "HashFunctions.hpp"
#include "Traits.hpp"

template <typename TFirst, typename TSecond>
struct Pair {
    TFirst first;
    TSecond second;
};

template <typename TFirst, typename TSecond>
struct Traits<Pair<TFirst, TSecond>> : public GenericTraits<Pair<TFirst, TSecond>> {
    static constexpr uint32_t hash(const Pair<TFirst, TSecond>& value)
    {
        return Traits<TFirst>::hash(value.first) ^ Traits<TSecond>::hash(value.second);
    }
};
