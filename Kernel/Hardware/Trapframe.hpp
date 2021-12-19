#pragma once

#include <Macaronlib/Common.hpp>

struct [[gnu::packed]] Trapframe {
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    uint32_t int_no, err_code;

    uint32_t eip, cs, eflags, useresp, ss;

    inline void push(uint32_t val)
    {
        useresp -= 4;
        *(uint32_t*)useresp = val;
    }

    inline uint32_t pop()
    {
        uint32_t val = *(uint32_t*)useresp;
        useresp += 4;
        return val;
    }
};