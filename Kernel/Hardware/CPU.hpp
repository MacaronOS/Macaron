#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel {

class CPU {
public:
    static auto& the()
    {
        static CPU the;
        return the;
    }

    void flush_tlb(uintptr_t virtual_address, size_t page_count);
};

}