#pragma once

#include <Macaronlib/Common.hpp>

struct [[gnu::packed]] Trapframe {
    uint32_t lr_usr;
    uint32_t sp_usr;
    uint32_t psr;
    uint32_t r[13];
    uint32_t pc;
};