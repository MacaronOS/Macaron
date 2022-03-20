#include <Devices/DeviceManager.hpp>
#include <Drivers/Disk/Ata.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/IO/Uart.hpp>
#include <Drivers/PCI/PCI.hpp>
#include <Drivers/PIT.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Filesystem/DevFS/DevFS.hpp>
#include <Filesystem/Ext2/Ext2.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Hardware/DescriptorTables/GDT.hpp>
#include <Hardware/DescriptorTables/IDT.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/Layout.hpp>
#include <Memory/Malloc.hpp>
#include <Memory/pmm.hpp>
#include <Memory/vmm.hpp>
#include <Multiboot.hpp>
#include <Tasking/Scheduler.hpp>
#include <Tasking/Syscalls/Syscalls.hpp>
#include <Time/TimeManager.hpp>

using namespace Kernel;
using namespace Drivers;
using namespace FS;
using namespace EXT2;
using namespace Syscalls;
using namespace Tasking;
using namespace Memory;
using namespace Logger;
using namespace Time;
using namespace Devices;

extern "C" void kernel_entry_point(multiboot_info_t* multiboot_structure)
{
    DescriptorTables::GDT::Setup();
    DescriptorTables::IDT::Setup();

    VgaTUI::Initialize();
    VgaTUI::Print("hello\n");

    Memory::SetupMalloc();
    PMM::the().initialize(multiboot_structure);

    SyscallsManager::initialize();

    // Setting up drivers
    PIT::the().initialize();
    PCI::the().initialize();

    DriverManager::the().add_driver(ata_0x1f0); // TODO: represent as a block device
    DriverManager::the().add_driver(uart); // TODO: represent as a character device
    DriverManager::the().install_all();

    DeviceManager::the().register_initial_devices();

    Ext2* ext2 = new Ext2(ata_0x1f0, VFS::the().file_storage());
    ext2->init();

    DevFS* devfs = new DevFS(VFS::the().file_storage());
    devfs->init();

    VFS::the().mount(VFS::the().root(), ext2->root(), "ext2");
    VFS::the().mount(VFS::the().root(), devfs->root(), "dev");

    if (!TimeManager::the().initialize()) {
        ASSERT_PANIC("Could not initialize TimeManager");
    }

    Scheduler::the().create_process("/ext2/System/System");
    Scheduler::the().run();
}
