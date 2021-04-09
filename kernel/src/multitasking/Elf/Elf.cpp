#include "Elf.hpp"
#include "elfstructs.hpp"

#include <wisterialib/String.hpp>
#include <wisterialib/memory.hpp>
#include <wisterialib/posix/defines.hpp>

#include <errors/KError.hpp>
#include <fs/vfs/vfs.hpp>
#include <memory/Region.hpp>
#include <memory/vmm.hpp>

namespace kernel::multitasking {

using namespace memory;

KErrorOr<Elf::ExecData> Elf::load_exec(const String& exec_path, uint32_t page_directory)
{
    auto& vfs = fs::VFS::the();
    auto& vmm = VMM::the();

    auto prog_fd_or_error = vfs.open(exec_path, 1);
    if (!prog_fd_or_error) {
        return prog_fd_or_error.error();
    }

    auto prog_fd = prog_fd_or_error.result();

    ElfHeader header {};
    vfs.read(prog_fd, &header, sizeof(header));

    if (
        header.ident[0] != 0x7f || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F') {
        return KError(ENOEXEC);
    }

    auto* ph_table = new ElfProgramHeader[header.phnum];
    vfs.lseek(prog_fd, header.phoff, SEEK_SET);
    vfs.read(prog_fd, ph_table, sizeof(ElfProgramHeader) * header.phnum);

    vmm.set_page_directory(page_directory);

    ExecData exec_data;

    for (size_t i = 0; i < header.phnum; i++) {
        if (ph_table[i].type == static_cast<uint32_t>(ElfProgramHeaderType::LOAD)) {

            size_t pages_count = (ph_table[i].memsz + ph_table[i].vaddr % PAGE_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;

            vmm.psized_allocate_space_from(
                page_directory,
                ph_table[i].vaddr / 4096,
                pages_count,
                Flags::User | Flags::Write | Flags::Present);

            exec_data.regions.push_back({
                .type = Region::Type::Allocated,
                .page = ph_table[i].vaddr / 4096,
                .pages = pages_count,
                .flags = Flags::User | Flags::Write | Flags::Present,
            });

            vfs.lseek(prog_fd, ph_table[i].offset, SEEK_SET);
            vfs.read(prog_fd, (void*)ph_table[i].vaddr, ph_table[i].filesz);

            memset((void*)(ph_table[i].vaddr + ph_table[i].filesz), 0, ph_table[i].memsz - ph_table[i].filesz);
        }
    }

    exec_data.entry_point = (uint32_t)header.entry;
    return exec_data;
}

}