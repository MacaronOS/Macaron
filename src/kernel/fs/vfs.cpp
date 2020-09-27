#include "vfs.hpp"

namespace kernel::fs {

uint32_t VFS::read(File& file, uint32_t offset, uint32_t size, uint8_t* buffer)
{
    if (file.read) {
        return file.read(file, offset, size, buffer);
    }
    return 0;
}

}