#pragma once

#include <fs/vfs/vfs.hpp>
#include <errors/KError.hpp>
#include <memory/Region.hpp>

#include <wisterialib/List.hpp>
#include <wisterialib/String.hpp>

namespace kernel::multitasking {

class Elf {
public:
    struct ExecData {
        uint32_t entry_point {};
        List<kernel::memory::Region> regions {};
    };
    static KErrorOr<Elf::ExecData> load_exec(const String& exec_path, uint32_t page_directory);
};

}