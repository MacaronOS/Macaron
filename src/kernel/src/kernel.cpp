#include "Logger.hpp"
#include "algo/Array.hpp"
#include "algo/Bitmap.hpp"
#include "algo/Deque.hpp"
#include "algo/Singleton.hpp"
#include "algo/StaticStack.hpp"
#include "algo/String.hpp"
#include "algo/Vector.hpp"
#include "assert.hpp"
#include "drivers/DriverManager.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/PIT.hpp"
#include "drivers/Uart.hpp"
#include "drivers/disk/Ata.hpp"
#include "fs/ext2/Ext2.hpp"
#include "fs/base/VNode.hpp"
#include "fs/ext2/ext2fs.hpp"
#include "fs/vfs/vfs.hpp"
#include "hardware/descriptor_tables.hpp"
#include "memory/Layout.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "multiboot.hpp"
#include "multitasking/TaskManager.hpp"
#include "shell/Shell.hpp"
#include "syscalls.hpp"
#include "tests/tests.hpp"
#include "drivers/pci/PCI.hpp"

using namespace kernel;
using namespace drivers;
using namespace Ata;
using namespace fs;
using namespace ext2;
using namespace multitasking;
using namespace syscalls;
using namespace memory;

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
    term_print("hello\n");
    kmalloc_init();
    InterruptManager::initialize();
    PMM::initialize<multiboot_info*>(multiboot_structure);
    VMM::initialize();
    SyscallsManager::initialize();

    // setting Drivers
    DriverManager::initialize();
    auto* ata = new Ata::Ata(0x1F0, true, DriverEntity::Ata0);
    auto* pit = new PIT();
    DriverManager::the().add_driver(*ata);
    DriverManager::the().add_driver(*pit);
    DriverManager::the().add_driver(*(new kernel::drivers::Keyboard()));
    DriverManager::the().add_driver(*(new kernel::drivers::Uart()));
    DriverManager::the().add_driver(*(new kernel::drivers::PCI()));
    DriverManager::the().install_all();

    // setting VFS
    VFS::initialize();
    Ext2* ext2 = new Ext2(*ata, VFS::the().file_storage());
    ext2->init();
    VFS::the().mount(VFS::the().root(), ext2->root(), "ext2");

#ifdef MISTIXX_TEST
    test_main();
#else
    asm volatile("sti");
    kernel::shell::run();

    // start up userspace process is going to be main

    TaskManager::initialize();
    TaskManager::the().create_process("/ext2/apps/main.app");
    TaskManager::the().run();
#endif
}
