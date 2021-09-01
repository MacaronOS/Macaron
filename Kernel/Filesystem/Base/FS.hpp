#pragma once
#include "VNode.hpp"

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

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

    virtual size_t getdents(VNode& directory, linux_dirent* dirp, size_t size) { return 0; }

    // creates a file inside file storage and returns it
    virtual VNode* create(VNode& directory, const String& name, FileType type, file_permissions_t perms) { return nullptr; }

    virtual bool erase(VNode& directory, const VNode& file) { return false; }

    virtual bool mmap(VNode& file, uint32_t addr, uint32_t size) { return false; };

    virtual bool ioctl(VNode& file, uint32_t request) { return false; };

    virtual bool can_read(VNode& vnode, uint32_t offset) { return false; };
};
}