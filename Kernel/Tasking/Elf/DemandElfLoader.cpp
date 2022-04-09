#include "DemandElfLoader.hpp"
#include "Elfstructs.hpp"
#include <Filesystem/Base/Inode.hpp>
#include <Tasking/Elf/ElfVMArea.hpp>
#include <Tasking/Process.hpp>

namespace Kernel::Tasking {

using namespace FileSystem;

KErrorOr<size_t> DemandElfLoader::load()
{
    auto inode = VFS::the().resolve_path(m_filename);
    if (!inode) {
        return KError(ENFILE);
    }

    ElfHeader header;
    inode->read_bytes(&header, sizeof(ElfHeader), 0);

    if (
        header.ident[0] != 0x7f || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F') {
        return KError(ENOEXEC);
    }

    Vector<ElfProgramHeader> ph_table(header.phnum);
    inode->read_bytes(ph_table.data(), header.phnum * sizeof(ElfProgramHeader), header.phoff);

    for (auto& header : ph_table) {
        if (header.type == static_cast<uint32_t>(ElfProgramHeaderType::LOAD)) {
            uint32_t flags = 0;
            if (header.flags & PF_R) {
                flags |= VM_READ;
            }
            if (header.flags & PF_W) {
                flags |= VM_WRITE;
            }
            if (header.flags & PF_X) {
                flags |= VM_EXEC;
            }

            auto area_status = m_process.memory_description().allocate_memory_area_from<ElfVMArea>(
                header.vaddr, header.memsz, flags);

            if (!area_status) {
                return area_status.error();
            }

            auto& area = *area_status.result();
            area.setup(inode, header.offset, header.vaddr, header.filesz, header.memsz);
        }
    }

    return static_cast<size_t>(header.entry);
}

}