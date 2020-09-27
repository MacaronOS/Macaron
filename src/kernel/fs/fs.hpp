#pragma once
#include "../types.hpp"

namespace kernel::fs {

#define MAX_FILE_NAME 128

struct dirent_t {
    char name[MAX_FILE_NAME];
    uint32_t inode;
};

enum class FileType {
    File,
    Directory,
    CharDevice,
    BlockDevice,
    Symlink,
    Mountpoint,
};

class File {
public:
    File() = default;
    ~File() = default;

    char name[MAX_FILE_NAME] {};
    uint32_t inode {};
    FileType type {};
    size_t size {};

    uint32_t (*read)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    uint32_t (*write)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    void (*open)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    void (*close)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    dirent_t (*readdir)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    File& (*finddir)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
};

class FS {
public:
    FS() = default;
    ~FS() = default;

    virtual uint32_t read(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { return 0; }
    virtual uint32_t write(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { return 0; }
    virtual void open(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
    virtual void close(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
    virtual dirent_t readdir(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
    virtual File& finddir(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) { }
};
}