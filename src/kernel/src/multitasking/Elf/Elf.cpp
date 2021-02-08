#include "Elf.hpp"
#include "elfstructs.hpp"

#include <algo/String.hpp>
#include <posix.hpp>
#include <fs/vfs.hpp>
#include <memory/vmm.hpp>
#include <errors/KError.hpp>

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

    ElfHeader header;
    vfs.read(prog_fd, &header, sizeof(header));

    if (
        header.ident[0] != 0x7f || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F') {
        return KError(ENOEXEC);
    }

    ElfProgramHeader* ph_table = new ElfProgramHeader[header.phnum];
    vfs.lseek(prog_fd, header.phoff, SEEK_SET);
    vfs.read(prog_fd, ph_table, sizeof(ElfProgramHeader) * header.phnum);

    uint32_t vaddr_end = 0;
    vmm.set_page_directory(page_directory);
    for (size_t i = 0; i < header.phnum; i++) {
        if (ph_table[i].type == static_cast<uint32_t>(ElfProgramHeaderType::LOAD)) {
            const size_t pages_count = (ph_table->memsz + PAGE_SIZE - 1) / PAGE_SIZE;
            vaddr_end = max(vaddr_end, ph_table[i].vaddr + pages_count * 4096);
            for (size_t page = 0; page < pages_count; page++) {
                vmm.create_frame(page_directory, ph_table[i].vaddr + page * 4096);
            }
            vfs.lseek(prog_fd, ph_table[i].offset, SEEK_SET);
            vfs.read(prog_fd, (void*)ph_table[i].vaddr, ph_table->filesz);
        }
    }

    ExecData exec_data;
    exec_data.entry_point = (uint32_t)header.entry;
    exec_data.vaddr_end = vaddr_end;

    return exec_data;
}

}