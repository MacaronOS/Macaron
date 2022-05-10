#pragma once

#include "../Common.hpp"

#ifdef __i386__
#include "Memcpy.x86.hpp"
#else
#include "Memcpy.aarch32.hpp"
#endif

[[gnu::always_inline]] inline void inline_memcpy(void* write, const void* read, size_t count)
{
    inline_memcpy_impl((char*)write, (const char*)read, count);
}
