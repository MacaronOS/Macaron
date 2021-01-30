#pragma once
#include "File.hpp"

#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../types.hpp"

namespace kernel::fs {
using algorithms::String;
using algorithms::Vector;

class FS {
public:
    FS() = default;
    ~FS() = default;

    virtual File& root() = 0;

    virtual uint32_t read(File& file, uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual uint32_t write(File& file, uint32_t offset, uint32_t size, void* buffer) { return 0; }

    virtual uint32_t truncate(File& file, uint32_t size) { return 0; }

    virtual void open(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
    virtual void close(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }

    // returns file by filename, if it exists
    virtual File* finddir(File& directory, const String& filename) { }

    // returns all filenames inside the directory
    virtual Vector<String> listdir(File& directory) { }

    // creates a file inside file storage and returns it
    virtual File& create(File& directory, const String& name, FileType type, file_permissions_t perms) { }

    virtual bool erase(File& directory, const File& file) { }
};
}