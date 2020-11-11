#include "algo/String.hpp"
#include "algo/Bitmap.hpp"
#include "algo/Vector.hpp"
#include "algo/StaticStack.hpp"
#include "algo/Array.hpp"
#include "algo/Deque.hpp"
#include "assert.hpp"
#include "descriptor_tables.hpp"
#include "drivers/DriverManager.hpp"
#include "drivers/disk/Ata.hpp"
#include "fs/File.hpp"
#include "fs/ext2fs.hpp"
#include "fs/Ext2.hpp"
#include "fs/vfs.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/regions.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "multiboot.hpp"
#include "syscalls.hpp"
#include "drivers/PIT.hpp"

using kernel::algorithms::Array;
using kernel::algorithms::Deque;
using kernel::algorithms::StaticStack;
using kernel::algorithms::Vector;
using kernel::drivers::DriverManager;
using kernel::drivers::PIT;
using kernel::drivers::Ata::Ata;
using kernel::fs::File;
using kernel::fs::FilePermission;
using kernel::fs::FileStorage;
using kernel::fs::FileType;
using kernel::fs::FS;
using kernel::fs::VFS;
using kernel::fs::ext2::Ext2;

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
    kmalloc_init();
    InterruptManager::initialize();

    // setting VMM
    VMM::initialize(get_pd_temp_location(), get_pt_temp_location()); 
    uint32_t pd = VMM::the().clone_page_directory();
    VMM::the().create_frame(pd, 0);
    VMM::the().set_page_directory(pd);

    // testing VMM
    int* a = (int*)22;
    term_printd(*a);

    // setting Drivers
    DriverManager::initialize();
    auto* ata = new Ata(0x1F0, true);
    auto* pit = new PIT();
    DriverManager::the().add_driver(*ata);
    DriverManager::the().add_driver(*pit);
    DriverManager::the().install_all();

    pit->register_callback({ 1000, []() { term_print("ticked"); } });

    // setting VFS
    VFS::initialize();
    Ext2* ext2 = new Ext2(*ata, VFS::the().file_storage());
    ext2->init();
    VFS::the().mount(VFS::the().root(), ext2->root(), "ext2");

    // testing VFS
    auto mounted = *VFS::the().finddir(VFS::the().root(), "ext2");

    Vector<String> dir = VFS::the().listdir(mounted);
    for (size_t i = 0; i < dir.size(); i++) {
        term_print(dir[i]);
        term_print("\n");
    }

    asm volatile("sti");
}