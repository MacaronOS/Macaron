#pragma once

#include <Macaronlib/Bitmap.hpp>
#include <Macaronlib/HashMap.hpp>

namespace Kernel::Memory {

class TranslationAllocator {
public:
    TranslationAllocator();

    template <typename T>
    T& allocate_tranlation_entity()
    {
        return *reinterpret_cast<T*>(allocate_bytes(sizeof(T)));
    }

    template <typename T>
    void deallocate_translation_entity(T& entity)
    {
        deallocate_bytes(&entity, sizeof(T));
    }

    template <typename T>
    T& get_translation_entity(uintptr_t physical_address)
    {
        return *reinterpret_cast<T*>(physical_to_virtual(physical_address));
    }

    uintptr_t physical_to_virtual(uintptr_t physical_address);
    uintptr_t virtual_to_physical(uintptr_t virtual_address);

private:
    void* allocate_bytes(size_t bytes);
    void deallocate_bytes(void* address, size_t bytes);

private:
    uintptr_t m_translation_area_start {};
    Bitmap m_allocated_pages {};
};

}