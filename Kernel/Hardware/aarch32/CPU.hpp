#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel {

// https://developer.arm.com/documentation/ddi0438/i/system-control/register-descriptions/configuration-base-address-register
uintptr_t read_peripheral_base()
{
    uint32_t cbar;
    asm("mrc p15, #4, %0, c15, c0, #0"
        : "=r"(cbar));
    return cbar & 0xffff8000;
}

}