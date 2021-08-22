#pragma once

#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Array.hpp>
#include <Wisterialib/Common.hpp>
#include <Wisterialib/Singleton.hpp>
#include <Wisterialib/StaticStack.hpp>

namespace Kernel::FS {

constexpr size_t FD_ALLOWED = 255;
typedef uint8_t fd_t;
typedef uint16_t mode_t;

struct Relation {
    VNode* directory;
    VNode* file;
};

class VFS : public Singleton<VFS> {

    class FileDescriptor {
    public:
        FileDescriptor() = default;

        int flags() const { return m_flags; }
        size_t offset() const { return m_offset; }
        VNode* vnode() { return m_vnode; }

        void set_flags(int flags) { m_flags |= flags; }

        void inc_offset(size_t offset) { m_offset += offset; }
        void dec_offset(size_t offset) { m_offset -= offset; }
        void set_offset(size_t offset) { m_offset = offset; }

        void set_file(VNode* vnode) { m_vnode = vnode; }

    private:
        VNode* m_vnode { nullptr };
        size_t m_offset { 0 };
        int m_flags { 0 };
    };

public:
    VFS();
    ~VFS() = default;

    VNode& root() { return *m_root; }
    VNodeStorage& file_storage() { return m_file_storage; }

    void mount(VNode& dir, VNode& appended_dir, const String& appended_dir_name);

    // posix like api functions
    KErrorOr<fd_t> open(const String& path, int flags, mode_t mode = 0);
    KError close(fd_t fd);
    KErrorOr<size_t> read(fd_t fd, void* buffer, size_t size);
    KErrorOr<size_t> write(fd_t fd, void* buffer, size_t size);
    KErrorOr<size_t> lseek(fd_t fd, size_t offset, int whence);
    KErrorOr<size_t> truncate(fd_t fd, size_t offset);
    KError mmap(fd_t fd, uint32_t addr, uint32_t size);
    KError ioctl(fd_t fd, uint32_t request);
    KError select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout);

    // sockets
    KErrorOr<fd_t> socket(int domain, int type, int protocol);
    KError bind(fd_t sockfd, const String& path);
    KError connect(fd_t sockfd, const String& path);

    // custom WisteriaOS api fuctions
    KErrorOr<size_t> file_size(fd_t fd);
    Vector<String> listdir(const String& path);
    bool can_read(fd_t fd);

    uint32_t read(VNode& file, uint32_t offset, uint32_t size, void* buffer);
    uint32_t write(VNode& file, uint32_t offset, uint32_t size, void* buffer);
    uint32_t file_size(VNode& file);

    VNode* finddir(VNode& directory, const String& filename);
    Vector<String> listdir(VNode& directory);

    VNode* create(VNode& directory, const String& name, FileType type, file_permissions_t perms);
    bool erase(VNode& directory, const VNode& file);

private:
    KErrorOr<VNode*> resolve_path(const String& path);
    FileDescriptor* get_file_descriptor(const fd_t fd);

private:
    KErrorOr<Relation> resolve_relation(const String& path);

private:
    VNode* m_root { nullptr };

    // Files
    VNodeStorage m_file_storage {};

    // VNode Descriptors
    FileDescriptor m_file_descriptors[FD_ALLOWED] {};
    StaticStack<fd_t, FD_ALLOWED> m_free_fds {};
};

}