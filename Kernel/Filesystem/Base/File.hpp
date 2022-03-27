#pragma once

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::FileSystem {

class File;

struct FileDescription {
    File* file {};
    size_t offset {};
    int flags;
};

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

typedef uint16_t FilePermissions;

class File {
public:
    virtual void open(FileDescription&) { }
    virtual bool can_read(FileDescription&) { return false; }
    virtual void read(void* buffer, size_t size, FileDescription& fd) { }
    virtual bool can_write(FileDescription&) { return false; }
    virtual void write(void* buffer, size_t size, FileDescription&) { }
    virtual void mmap(void* addr, size_t size) { }
    virtual void ioctl(uint32_t request) { }
    // TODO: implement as iterate
    virtual size_t getdents(linux_dirent* dirp, size_t size) { return 0; }
};

}