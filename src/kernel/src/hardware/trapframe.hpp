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

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.

    uint32_t int_no, err_code; // Interrupt number and error code

    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.

    void print()
    {
        kernel::Logger::Log() 
                            //   << "ds, es, fs, gs: " << ds << " " << es << " " << gs << "\n"
                              << "edi, esi, ebp: " << edi << " " << esi << " " << ebp << "\n"
                            //   << "esp, ebx, edx, ecx, eax: " << esp << " " << ebx << " " << ecx << " " << eax << "\n"
                            //   << "int_no, err_code: " << int_no << " " << err_code << "\n"
                              << "eip, cs, eflags: " << eip << " " << cs << " " << eflags << "\n"
                              << "useresp, ss: " << useresp << " " << ss << "\n";
    }
};