#include "monitor.h"
#include "descriptor_tables.h"
#include "multiboot.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/memory.h"
#include "assert.h"
#include "memory/regions.h"
#include "memory/kmalloc.h"
#include "drivers/ata.h"
#include "fs/ext2.h"

void kernel_main(multiboot_info_t* multiboot_structure) {
    init_descriptor_tables();
    term_init();
    pmm_init(multiboot_structure);
    vmm_init();
    kmalloc_init();

    term_print("Hello, World!\n");

    ata_t ata;
    ata_init(&ata, 0x1F0, true);
    ata_identify(&ata);

    ext2_init(&ata);
}