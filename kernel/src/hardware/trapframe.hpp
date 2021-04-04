#pragma once

#include "../monitor.hpp"

#include <wisterialib/common.hpp>
#include <Logger.hpp>

struct [[gnu::packed]] trapframe_t
{
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    uint32_t int_no, err_code;

    uint32_t eip, cs, eflags, useresp, ss;
};