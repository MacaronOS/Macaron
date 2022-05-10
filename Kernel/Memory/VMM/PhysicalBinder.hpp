#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

class PhysicalBinder {
public:
    PhysicalBinder(uintptr_t virtual_base_address);

    inline void* get() { return reinterpret_cast<void*>(m_virtual_base_address); }

    void bind_physical_page(uintptr_t physical_page_address);
    void unbind_physical_page();

private:
    uintptr_t m_virtual_base_address;
    volatile void* m_translation_entry {};
};

}