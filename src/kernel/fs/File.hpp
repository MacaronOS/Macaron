#pragma once
#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../algo/extras.hpp"
#include "../assert.hpp"
#include "../memory/kmalloc.hpp"
#include "../monitor.hpp"

#include "../types.hpp"
#include "ext2fs.hpp"

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
    UserRead = 0x100
};

typedef uint16_t file_permissions_t;

class File;
class Mountpoint {
public:
    Mountpoint(const String& name, File* file)
        : m_name(name)
        , m_file(file)
    {
    }
    Mountpoint(const Mountpoint& mp)
        : m_name(mp.m_name)
        , m_file(mp.m_file)
    {
    }
    Mountpoint(Mountpoint&& mp)
        : m_name(move(mp.m_name))
        , m_file(mp.m_file)
    {
    }
    Mountpoint operator=(const Mountpoint& mp)
    {
        m_name = mp.m_name;
        m_file = mp.m_file;
    }
    Mountpoint operator=(Mountpoint&& mp)
    {
        m_name = move(mp.m_name);
        m_file = mp.m_file;
    }

    String& name()
    {
        return m_name;
    }
    File& file()
    {
        return *m_file;
    }

private:
    String m_name;
    File* m_file;
};

// Represents an object that allows different filesystems communicate which each other.
class File {
public:
    File() = default;
    
    File(FS* fs, inode_t* inode_struct, uint32_t inode)
        : m_inode(inode)
        , m_inode_struct(inode_struct)
        , m_fs(fs)
    {
    }

    File(const File& file)
        : m_inode(file.m_inode)
        , m_inode_struct(file.m_inode_struct)
        , m_fs(file.m_fs)
        , m_mountpoints(file.m_mountpoints)
        , m_dirty(file.m_dirty)
        , m_ref_count(file.m_ref_count)
    {
    }

    File(File&& file)
        : m_inode(file.m_inode)
        , m_inode_struct(file.m_inode_struct)
        , m_fs(file.m_fs)
        , m_mountpoints(move(file.m_mountpoints))
        , m_dirty(file.m_dirty)
        , m_ref_count(file.m_ref_count)
    {
    }
    File operator=(const File& file)
    {
        m_inode = file.m_inode;
        m_inode_struct = file.m_inode_struct;
        m_fs = file.m_fs;
        m_mountpoints = file.m_mountpoints;
        m_dirty = file.m_dirty;
        m_ref_count = file.m_ref_count;
    }

    File operator=(File&& file)
    {
        m_inode = file.m_inode;
        m_inode_struct = file.m_inode_struct;
        m_fs = file.m_fs;
        m_mountpoints = move(file.m_mountpoints);
        m_dirty = file.m_dirty;
        m_ref_count = file.m_ref_count;
    }

    // Mount funcs
    void mount(Mountpoint& mountpoint)
    {
        m_mountpoints.push_back(mountpoint);
    }
    void mount(Mountpoint&& mountpoint)
    {
        m_mountpoints.push_back(move(mountpoint));
    }

    Vector<Mountpoint>& mounted_dirs() { return m_mountpoints; }

    // File System funcs
    FS* fs() const { return m_fs; }
    uint32_t inode() const { return m_inode; }
    inode_t* inode_struct() { return m_inode_struct; }
    inode_t* allocate_inode_struct() { m_inode_struct = new inode_t; return m_inode_struct; }
    size_t ref_count() { return m_ref_count; }

    void inc_ref_count() { m_ref_count++; }

private:
    FS* m_fs { nullptr };
    inode_t* m_inode_struct { nullptr };
    uint32_t m_inode { 0 };

    Vector<Mountpoint> m_mountpoints {};

    bool m_dirty { false };
    size_t m_ref_count { 0 };
};

class FileStorage {
public:
    FileStorage() = default;

    File& get(uint32_t inode, FS* fs, bool debug = false)
    {
        for (size_t i = 0; i < files.size(); i++) {
            if (files[i]->inode() == inode && files[i]->fs() == fs) {
                return *files[i];
            }
        }
        files.push_back(new File(fs, nullptr, inode));
        return *files.back();
    };

private:
    Vector<File*> files {};
};

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR 0x4
#define O_DIRECTORY 0x8
#define O_CREATE 0x10
#define O_TRUNC 0x20

class FileDescriptor {
public:
    FileDescriptor() = default;
    
    int flags() const { return m_flags; }
    size_t offset() const { return m_offset; }
    File* file() { return m_file; }

    void set_flags(int flags) { m_flags |= flags; }

    void inc_offset(size_t offset) { m_offset += offset; }
    void dec_offset(size_t offset) { m_offset -= offset; }
    void set_offset(size_t offset) { m_offset = offset; }

    void set_file(File* file) { m_file = file; }

private:
    File* m_file { nullptr };
    size_t m_offset { 0 };
    int m_flags { 0 };
};

}