#pragma once

#include <FileSystem/VFS/VFS.hpp>
#include <Libkernel/KError.hpp>
#include <Memory/Region.hpp>

#include <Macaronlib/List.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Tasking {

class Elf {
public:
    struct ExecData {
        uint32_t entry_point {};
        List<Kernel::Memory::Region> regions {};
    };
    static KErrorOr<Elf::ExecData> load_exec(const String& exec_path, uint32_t page_directory);
};

}