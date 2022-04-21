#pragma once

#include <Multiboot.hpp>

#include <Macaronlib/Bitmap.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

constexpr uint32_t FRAME_SIZE = 4096;

class PMM {
public:
    static PMM& the()
    {
        static PMM the {};
        return the;
    }

    void initialize(multiboot_info* multiboot_info);

    // allocates available frame
    // frame represents a FRAME_SIZE sized chunk of main memory
    uint32_t allocate_frame();

    uint32_t allocate_frames(uint32_t frames);

    // deallocates frame
    void free_frame(uint32_t frame);

    void occypy_frame(uint32_t frame);

    inline void occupy_range_sized(uint32_t addr_start, uint32_t size)
    {
        occupy_range(addr_start, addr_start + addr_start);
    }

private:
    // marks memory [left ; right] as occupied
    void occupy_range(uint32_t left, size_t right);

    // marks memory [left ; right] as free
    void free_range(uint32_t left, size_t right);

private:
    Bitmap m_pmmap; // contains the state of each memory block (0 - unused, 1 - used)
};

}
