#pragma once

#include <FileSystem/Base/FileSystem.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Libkernel/Logger.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Array.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/StaticStack.hpp>

namespace Kernel::FileSystem {

typedef uint8_t fd_t;
typedef uint16_t mode_t;

struct Relation {
    Dentry* directory;
    Dentry* file;
};

class VFS {
public:
    static VFS& the()
    {
        static VFS the;
        return the;
    }

    void init();

    Dentry* root_dentry() { return m_root_dentry; }

    Inode* resolve_path(const String& path);
    KError mount(const String& path, FileSystem& FileSystem);

    char* read_entire_file(const String& path);

    // posix like api functions
    KErrorOr<fd_t> open(const String& path, int flags, mode_t mode = 0);
    KError close(fd_t fd);
    KErrorOr<size_t> read(fd_t fd, void* buffer, size_t size);
    KErrorOr<size_t> write(fd_t fd, void* buffer, size_t size);
    KErrorOr<size_t> lseek(fd_t fd, size_t offset, int whence);
    KError mmap(fd_t fd, void* addr, uint32_t size);
    KError ioctl(fd_t fd, uint32_t request, void* arg);
    KError select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout);
    KErrorOr<size_t> getdents(fd_t fd, linux_dirent* dirp, size_t size);
    KError mkdir(const String& path);
    KErrorOr<String> ptsname(fd_t fd);

    // sockets
    KErrorOr<fd_t> socket(int domain, int type, int protocol);
    KError bind(fd_t sockfd, const String& path);
    KError connect(fd_t sockfd, const String& path);

    // custom MacaronOS api fuctions
    bool can_read(fd_t fd);

private:
    KErrorOr<Relation> resolve_relation(const String& path);

private:
    Dentry* m_root_dentry { nullptr };
};

}