#pragma once
#include "VNode.hpp"

#include <wisterialib/String.hpp>
#include <wisterialib/Vector.hpp>
#include <wisterialib/common.hpp>

#include <posix.hpp>

namespace kernel::fs {

class FS {
public:
    FS() = default;
    ~FS() = default;

    virtual VNode& root() = 0;

    virtual uint32_t read(VNode& file, uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual uint32_t write(VNode& file, uint32_t offset, uint32_t size, void* buffer) { return 0; }

    virtual uint32_t truncate(VNode& file, uint32_t size) { return 0; }

    virtual void open(VNode& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
    virtual void close(VNode& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }

    // returns file by filename, if it exists
    virtual VNode* finddir(VNode& directory, const String& filename) { return nullptr; }

    // returns all filenames inside the directory
    virtual Vector<String> listdir(VNode& directory) { return {}; }

    // creates a file inside file storage and returns it
    virtual VNode* create(VNode& directory, const String& name, FileType type, file_permissions_t perms) { return nullptr; }

    virtual bool erase(VNode& directory, const VNode& file) { return false; }

    virtual bool mmap(VNode& file, uint32_t addr, uint32_t size) { return false; };
};
}