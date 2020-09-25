#include "assert.hpp"
#include "descriptor_tables.hpp"
#include "drivers/ata.hpp"
#include "fs/ext2.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/regions.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "multiboot.hpp"

extern "C" void kernel_main(multiboot_info_t* multiboot_structure)
{
    init_descriptor_tables();
    term_init();
    term_print("Hello, World!\n");

    pmm_init(multiboot_structure);
    vmm_init();
    kmalloc_init();

    ata_t ata;
    ata_init(&ata, 0x1F0, true);
    ata_identify(&ata);

    ext2_init(&ata);
    ext2_read_inode(&ata, 2);
}