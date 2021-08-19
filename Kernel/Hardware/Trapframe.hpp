#pragma once

#include <Libkernel/Logger.hpp>
#include <Wisterialib/common.hpp>

struct [[gnu::packed]] Trapframe
{
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    uint32_t int_no, err_code;

    uint32_t eip, cs, eflags, useresp, ss;
};