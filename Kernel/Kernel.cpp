#include <Devices/DeviceManager.hpp>
#include <Drivers/Disk/Ata.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/IO/Uart.hpp>
#include <Drivers/PCI/PCI.hpp>
#include <Drivers/PIT.hpp>
#include <FileSystem/Dev/DevFileSystem.hpp>
#include <FileSystem/Ext2/Ext2FileSystem.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Hardware/DescriptorTables/GDT.hpp>
#include <Hardware/DescriptorTables/IDT.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Init.hpp>
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
using namespace FileSystem;
using namespace Ext2;
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
    VgaTUI::Print("Starting up Macaron OS kernel...\n");

    Memory::SetupMalloc();

    Libkernel::CallConstructors();

    PMM::the().initialize(multiboot_structure);

    SyscallsManager::initialize();

    // Setting up drivers
    PIT::the().initialize();
    PCI::the().initialize();

    DriverManager::the().add_driver(ata_0x1f0); // TODO: represent as a block device
    DriverManager::the().add_driver(uart); // TODO: represent as a character device
    DriverManager::the().install_all();

    DeviceManager::the().register_initial_devices();

    VFS::the().init();

    auto ext2 = new Ext2FileSystem(ata_0x1f0);
    ext2->init();

    devfs.init();

    VFS::the().mount("/", *ext2);
    VFS::the().mount("/dev", devfs);

    if (!TimeManager::the().initialize()) {
        ASSERT_PANIC("Could not initialize TimeManager");
    }

    Scheduler::the().create_process("/System/System");
    Scheduler::the().run();
}
