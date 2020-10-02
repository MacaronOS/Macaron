#pragma once
#include "../algo/String.hpp"
#include "../types.hpp"

namespace kernel::fs {
using algorithms::String;

enum class FileType {
    File,
    Directory,
    CharDevice,
    BlockDevice,
    Symlink,
    Mountpoint,
    NOTAFILE,
};

class File {
public:
    File() = default;
    File(const FileType& type)
        : m_type(type)
    {
    }
    File(uint32_t inode)
        : m_inode(inode)
    {
    }
    File(uint32_t inode, const String& name)
        : m_inode(inode)
        , m_name(name)
    {
    }

    ~File() = default;

    String name() const { return m_name; }
    uint32_t inode() const { return m_inode; }
    FileType type() const { return m_type; }
    size_t size() const { return m_size; }

    uint32_t (*read)(File& file, uint32_t offset, uint32_t size, void* buffer) {};
    uint32_t (*write)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    void (*open)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    void (*close)(File& file, uint32_t offset, uint32_t size, uint8_t* buffer) {};
    File (*finddir)(const File& directory, const String& filename) {};

private:
    String m_name {};
    uint32_t m_inode {};
    FileType m_type {};
    size_t m_size {};
};

}