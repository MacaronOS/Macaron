#pragma once
#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../types.hpp"

namespace kernel::fs {
using algorithms::String;
using algorithms::Vector;

class FS;

enum class FileType {
    FIFO = 0x1000,
    CharDevice = 0x2000,
    Directory = 0x4000,
    BlockDevice = 0x6000,
    File = 0x8000,
    Symlink = 0xA000,
    Socket = 0xC000,
    Mountpoint,
    NOTAFILE,
};

enum class FilePermission {
    Execute = 0x001,
    Write = 0x002,
    Read = 0x004,
    GroupExecute = 0x008,
    GroupWrite = 0x010,
    GroupRead = 0x020,
    UserExecute = 0x040,
    UserWrite = 0x080,
    UserRead = 0x100,
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
    void set_name(const String& s) { m_name = s; }

    uint32_t inode() const { return m_inode; }
    void set_inode(uint32_t inode) { m_inode = inode; }

    FileType type() const { return m_type; }
    void set_type(const FileType& type) { m_type = type; }

    size_t size() const { return m_size; }

    void set_permission(const FilePermission& permission) { m_permissions_holder |= static_cast<uint32_t>(permission); }
    bool check_permission(const FilePermission& permission) { return m_permissions_holder &= static_cast<uint32_t>(permission); }
    uint16_t get_permissions() { return m_permissions_holder; }

    void mount(File* file)
    {
        m_mounted_dirs.push_back(file);
    }

    const Vector<File*> mounted_dirs() const { return m_mounted_dirs; }

    void bind_fs(FS* fs) {
        m_fs = fs;
    }

    FS* fs() const { return m_fs; }

private:
    String m_name {};
    uint32_t m_inode {};
    FileType m_type {};
    uint16_t m_permissions_holder {};
    size_t m_size {};

    Vector<File*> m_mounted_dirs {};

    FS* m_fs {};
};

}