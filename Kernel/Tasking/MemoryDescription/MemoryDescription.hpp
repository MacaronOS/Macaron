#pragma once

#include "VMArea.hpp"
#include <Libkernel/KError.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/List.hpp>
#include <Memory/VMM/VMM.hpp>

namespace Kernel::Tasking {

using namespace Memory;

class TranslationTableRestorer {
public:
    TranslationTableRestorer()
        : m_last_translation_table(VMM::the().current_translation_table())
    {
    }

    ~TranslationTableRestorer()
    {
        VMM::the().set_translation_table(m_last_translation_table);
    }

private:
    uintptr_t m_last_translation_table;
};

/*
MemoryDescription is an analogue of Linux mm_struct.
It belongs to a Process and is used to keep a list of mapped areas of virtual memory.
*/

class MemoryDescription {
public:
    using Iterator = List<VMArea*>::Iterator;

    MemoryDescription();
    MemoryDescription(uint32_t memory_descriptor)
        : m_memory_descriptor(memory_descriptor)
    {
    }

    inline uint32_t memory_descriptor() const { return m_memory_descriptor; }

    void fork_from(MemoryDescription& other);
    void free_memory();
    VMArea* find_memory_area_for(size_t address);

    template <typename T>
    KErrorOr<T*> allocate_memory_area(size_t size, uint32_t flags, bool allow_higher_half = false)
    {
        size = (size + 4096 - 1) / 4096 * 4096;

        if (m_memory_areas.empty()) {
            m_memory_areas.push_back(new T(*this, 4096, 4096 + size, flags));
            return static_cast<T*>(m_memory_areas.back());
        }

        if (allow_higher_half || m_memory_areas.back()->vm_end() + size <= 0xC0000000) {
            m_memory_areas.push_back(new T(
                *this,
                m_memory_areas.back()->vm_end(),
                m_memory_areas.back()->vm_end() + size,
                flags));

            return static_cast<T*>(m_memory_areas.back());
        }

        return KError(ENOMEM);
    }

    template <typename T>
    KErrorOr<T*> allocate_memory_area_from(size_t start, size_t size, uint32_t flags)
    {
        size_t end = (start + size + 4096 - 1) / 4096 * 4096;
        start = start / 4096 * 4096;

        Iterator first_greater_area;

        for (first_greater_area = m_memory_areas.begin();
             first_greater_area != m_memory_areas.end();
             first_greater_area++) {

            auto& area = *first_greater_area;

            if (area->vm_end() > start) {
                if (area->vm_start() >= end) {
                    break;
                }
                return KError(ENOMEM);
            }
        }

        return static_cast<T*>(*m_memory_areas.insert_before(first_greater_area, new T(*this, start, end, flags)));
    }

    enum class FreeMemoryArea {
        Yes,
        No,
    };

    template <typename Callback>
    void free_memory_areas(Callback callback)
    {
        TranslationTableRestorer _;
        VMM::the().set_translation_table(m_memory_descriptor);

        auto area_it = m_memory_areas.rbegin();
        while (area_it != m_memory_areas.rend()) {
            if (callback(**area_it) == FreeMemoryArea::Yes) {
                free_memory_area(area_it);
                area_it = m_memory_areas.remove(area_it);
                continue;
            }
            --area_it;
        }
    }

private:
    void free_memory_area(Iterator area_it);

public:
    List<VMArea*> m_memory_areas {};
    uint32_t m_memory_descriptor;
};

// A special memory description object used to store memory mappings that are
// above the HIGHER_HALF_OFFSET.
// Currently this object is used for keeping a shared area that processes use
// for calling signals.
extern MemoryDescription kernel_memory_description;
void init_kernel_memory_description();

}