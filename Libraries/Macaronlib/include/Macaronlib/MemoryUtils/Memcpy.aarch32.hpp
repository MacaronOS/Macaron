#pragma once

#include "../Common.hpp"

[[gnu::always_inline]] inline void inline_memcpy_impl(char* write, const char* read, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        *(write + i) = *(read + i);
    }
}