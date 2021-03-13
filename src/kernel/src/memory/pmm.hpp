#pragma once

#include <types.hpp>
#include <multiboot.hpp>

#include <algo/Bitmap.hpp>
#include <algo/Singleton.hpp>

namespace kernel::memory {

constexpr uint32_t FRAME_SIZE = 4096;

class PMM : public Singleton<PMM> {
public:
    PMM(multiboot_info* multiboot_info);

    // allocates available frame
    // frame represents a FRAME_SIZE sized chunk of main memory
    uint32_t allocate_frame();
    // deallocates frame
    void free_frame(uint32_t frame);

    void occypy_frame(uint32_t frame);

private:
    // marks memory [left ; right] as occupied
    void occupy_range(uint32_t left, size_t right);

    // marks memory [left ; right] as free
    void free_range(uint32_t left, size_t right);

private:
    multiboot_info* m_multiboot_info; // initial disk layout information, provided by GRUB
    Bitmap m_pmmap; // contains the state of each memory block (0 - unused, 1 - used)
};

}
