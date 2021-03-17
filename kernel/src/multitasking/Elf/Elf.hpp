#pragma once
#include <wisterialib/String.hpp>
#include <fs/vfs/vfs.hpp>
#include <errors/KError.hpp>

namespace kernel::multitasking {

class Elf {
public:
    Elf() = default;

    struct ExecData {
        uint32_t entry_point; // needed by eip 
        uint32_t vaddr_end; // needed to calculate the heap and stack
    };
    KErrorOr<Elf::ExecData> load_exec(const String& exec_path, uint32_t page_directory);
};

}