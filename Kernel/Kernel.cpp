#include <Libkernel/Logger.hpp>
#include <Libkernel/Assert.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/IO/Keyboard.hpp>
#include <Drivers/IO/Mouse.hpp>
#include <Drivers/PIT.hpp>
#include <Drivers/IO/Uart.hpp>
#include <Drivers/Disk/Ata.hpp>
#include <Drivers/PCI/PCI.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Filesystem/DevFS/DevFS.hpp>
#include <Filesystem/Ext2/Ext2.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Time/TimeManager.hpp>
#include <Memory/Layout.hpp>
#include <Memory/Malloc.hpp>
#include <Memory/pmm.hpp>
#include <Memory/vmm.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Multiboot.hpp>
#include <Tasking/TaskManager.hpp>
#include <Hardware/DescriptorTables/GDT.hpp>
#include <Hardware/DescriptorTables/IDT.hpp>
#include <Tasking/Syscalls/Syscalls.hpp>
#include <Tasking/SharedBuffers/SharedBufferStorage.hpp>

using namespace Kernel;
using namespace Drivers;
using namespace FS;
using namespace EXT2;
using namespace Syscalls;
using namespace Tasking;
using namespace Memory;
using namespace Logger;
using namespace Time;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

extern "C" void KernelEntryPoint(multiboot_info_t* multiboot_structure)
{
    DescriptorTables::GDT::Setup();
    DescriptorTables::IDT::Setup();

    VgaTUI::Initialize();
    VgaTUI::Print("hello\n");
    
    Memory::SetupMalloc();

    InterruptManager::initialize();
    PMM::initialize<multiboot_info*>(multiboot_structure);
    VMM::initialize();
    SyscallsManager::initialize();

    // setting Drivers/
    DriverManager::initialize();
    auto* ata = new Ata(0x1F0, true, DriverEntity::Ata0);
    auto* pit = new PIT();
    DriverManager::the().add_driver(ata);
    DriverManager::the().add_driver(pit);
    DriverManager::the().add_driver(new Kernel::Drivers::Keyboard());
    DriverManager::the().add_driver(new Kernel::Drivers::Uart());
    DriverManager::the().add_driver(new Kernel::Drivers::PCI());
    DriverManager::the().add_driver(new Kernel::Drivers::Mouse());
    DriverManager::the().install_all();

    TimeManager::initialize();

    // setting VFS
    VFS::initialize();

    Ext2* ext2 = new Ext2(*ata, VFS::the().file_storage());
    ext2->init();

    DevFS* devfs = new DevFS(VFS::the().file_storage());
    devfs->init();

    VFS::the().mount(VFS::the().root(), ext2->root(), "ext2");
    VFS::the().mount(VFS::the().root(), devfs->root(), "dev");

    SharedBufferStorage::initialize();

    TaskManager::initialize();
    TaskManager::the().create_process("/ext2/System/System");
    TaskManager::the().run();
}
