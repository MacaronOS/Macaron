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

void kernel_main(multiboot_info_t* multiboot_structure) {
    init_descriptor_tables();
    term_init();
    pmm_init(multiboot_structure);
    vmm_init();
    kmalloc_init();

    term_print("Hello, World!\n");

    // testing up ata driver
    ata_t ata;
    ata_init(&ata, 0x1F0, true);
    ata_identify(&ata);

    uint16_t buf[256];
    memset(&buf, 1, 256 * 2);

    ata_write28(&ata, 0, 1, &buf);

    uint16_t buf2[256];
    memset(&buf2, 2, 256 * 2);

    ata_read28(&ata, 0, 1, &buf2);

    for (size_t i = 0; i < 256; i++) {
        term_printd(buf2[i]);
    }
}