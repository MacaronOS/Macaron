#pragma once
#include "fs.hpp"

namespace kernel::fs {

class VFS : public FS {
public:
    uint32_t read(File& file, uint32_t offset, uint32_t size, void* buffer) override;
};

}