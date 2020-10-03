#include "algo/String.hpp"
#include "algo/Bitmap.hpp"
#include "assert.hpp"
#include "descriptor_tables.hpp"
#include "drivers/DriverManager.hpp"
#include "drivers/disk/Ata.hpp"
#include "fs/File.hpp"
#include "fs/ext2.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/regions.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "multiboot.hpp"

using kernel::fs::File;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

extern "C" void kernel_main(multiboot_info_t* multiboot_structure)
{
    init_descriptor_tables();
    term_init();
    term_print("Hello, World!\n");

    pmm_init(multiboot_structure);
    vmm_init();
    kmalloc_init();

    kernel::drivers::DriverManager driver_manager = kernel::drivers::DriverManager();

    kernel::drivers::Ata::Ata ata = kernel::drivers::Ata::Ata(0x1F0, true);

    driver_manager.add_driver(ata);
    driver_manager.install_all();

    kernel::fs::ext2::Ext2 ext2 = kernel::fs::ext2::Ext2(ata);
    ext2.init();

    ext2.read_directory(2);

    term_print("\nfile: ");
    term_print(ext2.finddir(File(2), "file.txt").name());
    term_print("\n");
}