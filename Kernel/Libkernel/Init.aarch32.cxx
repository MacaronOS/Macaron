
#include <Macaronlib/Common.hpp>
#include <Memory/Layout.hpp>
#include <Multiboot.hpp>

extern "C" uint32_t _boot_start;
extern "C" uint32_t _boot_end;

extern "C" {

volatile multiboot_info_t multiboot_structure;
volatile multiboot_memory_map_t multiboot_memory_map[2];

void emulate_multiboot()
{
    multiboot_structure.mem_upper = 0x10000000 / 1024; // In KBs.
    // Subtract HIGHER_HALF_OFFSET from the memory map location to make PMM code generic.
    multiboot_structure.mmap_addr = (uintptr_t)&multiboot_memory_map - Kernel::Memory::HIGHER_HALF_OFFSET;
    multiboot_structure.mmap_length = sizeof(multiboot_memory_map);

    multiboot_memory_map[0].addr = 0x80000000;
    multiboot_memory_map[0].len = 0x10000000;
    multiboot_memory_map[0].type = MULTIBOOT_MEMORY_AVAILABLE;
    multiboot_memory_map[0].size = sizeof(multiboot_memory_map_t);

    multiboot_memory_map[1].addr = (uintptr_t)&_boot_start;
    multiboot_memory_map[1].len = (uintptr_t)&_boot_end - (uintptr_t)&_boot_start;
    multiboot_memory_map[1].type = MULTIBOOT_MEMORY_RESERVED;
    multiboot_memory_map[1].size = sizeof(multiboot_memory_map_t);
}
}