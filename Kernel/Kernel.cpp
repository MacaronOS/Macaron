#include <Devices/DeviceManager.hpp>
#include <FileSystem/Dev/DevFileSystem.hpp>
#include <FileSystem/Ext2/Ext2FileSystem.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Hardware/Init.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Init.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/Layout/Layout.hpp>
#include <Memory/Malloc.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Memory/pmm.hpp>
#include <Multiboot.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/Scheduler/Scheduler.hpp>
#include <Tasking/Syscalls/Syscalls.hpp>
#include <Time/TimeManager.hpp>

using namespace Kernel;
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
    initialize_hardware();

    Memory::SetupMalloc();

    Libkernel::CallConstructors();

    PMM::the().initialize(multiboot_structure);

    Tasking::init_kernel_memory_description();

    SyscallsManager::initialize();

    DeviceManager::the().install_acknowledged_drivers();
    DeviceManager::the().register_virtual_devices();

    VFS::the().init();

    auto hda0 = DeviceManager::the().get_block_device(3, 0);
    if (!hda0) {
        ASSERT_PANIC("No hard drive");
    }
    auto ext2 = new Ext2FileSystem(*hda0);
    ext2->init();

    devfs.init();

    VFS::the().mount("/", *ext2);
    VFS::the().mount("/dev", devfs);

    if (!TimeManager::the().initialize()) {
        ASSERT_PANIC("Could not initialize TimeManager");
    }

    Scheduler::the().initialize();
    Scheduler::the().run();
}
