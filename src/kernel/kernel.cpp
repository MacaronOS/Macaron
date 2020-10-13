#include "algo/String.hpp"
#include "algo/Bitmap.hpp"
#include "algo/Vector.hpp"
#include "algo/StaticStack.hpp"
#include "algo/Array.hpp"
#include "assert.hpp"
#include "descriptor_tables.hpp"
#include "drivers/DriverManager.hpp"
#include "drivers/disk/Ata.hpp"
#include "fs/File.hpp"
#include "fs/ext2.hpp"
#include "fs/Ext2.hpp"
#include "fs/vfs.hpp"
#include "memory/kmalloc.hpp"
#include "memory/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/regions.hpp"
#include "memory/vmm.hpp"
#include "monitor.hpp"
#include "multiboot.hpp"

using kernel::algorithms::Array;
using kernel::algorithms::StaticStack;
using kernel::algorithms::Vector;
using kernel::fs::File;
using kernel::fs::FilePermission;
using kernel::fs::FileType;
using kernel::fs::FileStorage;
using kernel::fs::VFS;
using kernel::fs::FS;
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
    vmm_init();
    kmalloc_init();

    kernel::drivers::DriverManager driver_manager = kernel::drivers::DriverManager();

    kernel::drivers::Ata::Ata ata = kernel::drivers::Ata::Ata(0x1F0, true);

    VFS vfs = VFS();
    Ext2 ext2 = Ext2(ata, vfs.file_storage());
    ext2.init();

    vfs.mount(vfs.root(), ext2.root(), "ext2");
    auto mounted = *vfs.finddir(vfs.root(), "ext2");

    Vector<String> dir = vfs.listdir(mounted);
    for (size_t i = 0; i < dir.size(); i++) {
        term_print(dir[i]);
        term_print("\n");
    }
}