#pragma once

#include <Hardware/CPU.hpp>
#include <Multiboot.hpp>

#include <Macaronlib/Bitmap.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

class PMM {
public:
    static PMM& the()
    {
        static PMM the {};
        return the;
    }

    void initialize(multiboot_info* multiboot_info);

    // Allocates an available frame.
    // Frame represents a page sized chunk of main memory.
    size_t allocate_frame();
    // Alocate "frames" number of sequential frames.
    size_t allocate_frames(size_t frames);
    // Deallocates a frame.
    void free_frame(size_t frame);
    // Marks a frame as occupied.
    void occypy_frame(size_t frame);

    inline void occupy_range_sized(uintptr_t addr_start, size_t size)
    {
        occupy_range(addr_start, addr_start + size);
    }

    inline void free_range_sized(uintptr_t addr_start, size_t size)
    {
        free_range(addr_start, addr_start + size);
    }

private:
    // Marks memory [start ; end) as occupied.
    void occupy_range(uintptr_t start, uintptr_t end);
    // Marks memory [start ; end) as free.
    void free_range(uintptr_t start, uintptr_t end);

private:
    Bitmap m_pmmap; // contains the state of each memory block (0 - unused, 1 - used)
};

}
