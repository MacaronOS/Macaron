#pragma once

#include "../Common.hpp"

[[gnu::always_inline]] inline void inline_memcpy_impl(char* write, const char* read, size_t count)
{
header:
    switch (count) {
    case 0:
        return;
    case 1:
        *write = *read;
        return;
    case 2:
        *(uint16_t*)write = *(uint16_t*)read;
        return;
    case 3:
        *(uint16_t*)write = *(uint16_t*)read;
        *(write + sizeof(uint16_t)) = *(read + sizeof(uint16_t));
        return;
    case 4:
        *(uint32_t*)write = *(uint32_t*)read;
        return;
    }

    if (count < 12) {
        *(uint32_t*)write = *(uint32_t*)read;
        count -= sizeof(uint32_t);
        write += sizeof(uint32_t);
        read += sizeof(uint32_t);
        goto header;
    }

    if (!((size_t)write & 0x3) && !((size_t)read & 0x3)) {
        size_t src = (size_t)read;
        size_t dest = (size_t)write;
        size_t count_movsd = count / sizeof(uint32_t);
        size_t count_movsd_cp = count_movsd;

        asm volatile(
            "rep movsd\n"
            : "=S"(src), "=D"(dest)
            : "S"(src), "D"(dest), "c"(count_movsd)
            : "memory");

        count -= count_movsd_cp * sizeof(uint32_t);
        write += count_movsd_cp * sizeof(uint32_t);
        read += count_movsd_cp * sizeof(uint32_t);

        goto header;
    }

    asm volatile(
        "rep movsb\n" ::"S"(read), "D"(write), "c"(count)
        : "memory");
}