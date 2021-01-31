#include "algo/Array.hpp"
#include "algo/Bitmap.hpp"
#include "algo/Deque.hpp"
#include "algo/StaticStack.hpp"
#include "algo/String.hpp"
#include "algo/Vector.hpp"
#include "algo/Singleton.hpp"
#include "assert.hpp"
#include "hardware/descriptor_tables.hpp"
#include "drivers/DriverManager.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/PIT.hpp"
#include "drivers/disk/Ata.hpp"
#include "fs/Ext2.hpp"
#include "fs/File.hpp"
#include "fs/ext2fs.hpp"
#include "fs/vfs.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/regions.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "syscalls.hpp"
#include "multiboot.hpp"
#include "multitasking/TaskManager.hpp"
#include "shell/Shell.hpp"
#include "tests/tests.hpp"

using kernel::algorithms::Array;
using kernel::algorithms::Deque;
using kernel::algorithms::StaticStack;
using kernel::algorithms::Vector;
using kernel::drivers::DriverEntity;
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
using kernel::multitasking::TaskManager;
using kernel::syscalls::SyscallsManager;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

extern "C" void sys_printd(int);
extern "C" void switch_to_user_mode();

void test1()
{
    while (true) {
        term_print("test kernel thread: ");
        sys_printd(1);
        term_printd(1);
        term_print("\n");
    }

    // exit() - destroy thread
    // destroy thread
}

void test2()
{
    while (true) {
        term_print("test kernel thread: ");
        term_printd(2);
        term_print("\n");
    }
}

extern "C" void kernel_main(multiboot_info_t* multiboot_structure)
{
    init_descriptor_tables();
    term_init();
    pmm_init(multiboot_structure);
    kmalloc_init();
    InterruptManager::initialize();
    SyscallsManager::initialize();

    // setting VMM
    VMM::initialize();

    // setting Drivers
    DriverManager::initialize();
    auto* ata = new Ata(0x1F0, true, DriverEntity::Ata0);
    auto* pit = new PIT();
    DriverManager::the().add_driver(*ata);
    DriverManager::the().add_driver(*pit);
    DriverManager::the().add_driver(*(new kernel::drivers::Keyboard()));
    DriverManager::the().install_all();

    // setting VFS
    VFS::initialize();
    Ext2* ext2 = new Ext2(*ata, VFS::the().file_storage());
    ext2->init();
    VFS::the().mount(VFS::the().root(), ext2->root(), "ext2");

    asm volatile("sti");

#ifdef MISTIXX_TEST
    test_main();
#else
    kernel::shell::run();

    // start up userspace process is going to be main 
    
    // TaskManager::initialize();
    // TaskManager::the().create_process("/ext2/apps/main.app");
    // TaskManager::the().run();
#endif
}
