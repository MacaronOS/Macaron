#include "VFS.hpp"

#include <Devices/PTY/PTYMaster.hpp>
#include <Devices/PTY/PTYSlave.hpp>
#include <FileSystem/Base/DentryCache.hpp>
#include <FileSystem/Base/Inode.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>
#include <Tasking/Net/LocalSocket.hpp>
#include <Tasking/Scheduler/Scheduler.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Memory.hpp>
#include <Macaronlib/Runtime.hpp>

namespace Kernel::FileSystem {

using namespace Net;
using namespace Devices;
using namespace Tasking;

void VFS::init()
{
    s_dentry_cache.init();

    Dentry root_dentry(nullptr, "");
    m_root_dentry = s_dentry_cache.put(move(root_dentry));
}

Inode* VFS::resolve_path(const String& path)
{
    auto relation = resolve_relation(path);
    if (!relation) {
        return nullptr;
    }

    auto file = relation.result().file;
    if (!file) {
        return nullptr;
    }

    return file->inode();
}

KErrorOr<fd_t> VFS::open(const String& path, int flags, mode_t mode)
{
    auto free_fd = Scheduler::the().current_process().file_descriptions().allocate();
    if (!free_fd) {
        return free_fd;
    }

    auto relation = resolve_relation(path);

    if (!relation) {
        return relation.error();
    }

    auto file_dentry = relation.result().file;
    auto directory_dentry = relation.result().directory;

    if (!file_dentry) {
        if ((mode & O_CREAT)) {
            auto inode = directory_dentry->inode()->create(path.split("/").back(), FileType::File, flags);
            file_dentry->set_inode(inode);
        } else {
            return KError(ENOENT);
        }
    }

    auto file_description_result = Scheduler::the().current_process().file_descriptions().lookup(free_fd.result());
    if (!file_description_result) {
        return file_description_result.error();
    }

    auto file_description = file_description_result.result();
    file_description->flags = flags;
    file_description->offset = 0;
    file_dentry->inode()->inode_open(*file_description);
    if (file_description->file) {
        file_description->file->open(*file_description);
    }

    return free_fd;
}

KError VFS::close(const fd_t fd)
{
    return Scheduler::the().current_process().file_descriptions().free(fd);
}

KError VFS::mount(const String& path, FileSystem& FileSystem)
{
    auto relation = resolve_relation(path);
    if (!relation) {
        return relation.error();
    }

    auto file = relation.result().file;
    if (!file) {
        return KError(ENOENT);
    }

    if (file->inode()) {
        return KError(EEXIST);
    }

    file->set_inode(FileSystem.root());
    file->update_count(1);

    return KError(0);
}

char* VFS::read_entire_file(const String& path)
{
    auto inode = resolve_path(path);
    if (!inode) {
        return nullptr;
    }

    size_t size = inode->size();
    auto buffer = (char*)malloc(size);

    FileDescription fd;
    inode->inode_open(fd);
    fd.file->read(buffer, size, fd);

    return buffer;
}

KErrorOr<size_t> VFS::read(fd_t fd, void* buffer, size_t size)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    auto file = file_descr->file;
    if (!file) {
        return KError(ENOENT);
    }

    if (!file->can_read(*file_descr)) {
        Scheduler::the().block_current_thread_with<ReadBlocker>(*file_descr);
    }

    size_t offset = file_descr->offset;
    file->read(buffer, size, *file_descr);
    return file_descr->offset - offset;
}

KErrorOr<size_t> VFS::write(fd_t fd, void* buffer, size_t size)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    auto file = file_descr->file;
    if (!file) {
        return KError(ENOENT);
    }

    if (!file->can_write(*file_descr)) {
        Scheduler::the().block_current_thread_with<WriteBlocker>(*file_descr);
    }

    size_t offset = file_descr->offset;
    file->write(buffer, size, *file_descr);
    return file_descr->offset - offset;
}

KErrorOr<size_t> VFS::lseek(fd_t fd, size_t offset, int whence)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    switch (whence) {
    case SEEK_SET: {
        file_descr->offset = offset;
        break;
    }
    case SEEK_CUR: {
        file_descr->offset += offset;
        break;
    }
    default:
        return KError(EINVAL);
    }

    return file_descr->offset;
}

KError VFS::mmap(fd_t fd, void* addr, uint32_t size)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    if (!file_descr->file) {
        KError(ENOENT);
    }

    file_descr->file->mmap(addr, size);
    return KError(0);
}

KError VFS::ioctl(fd_t fd, uint32_t request, void* arg)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    if (!file_descr->file) {
        KError(ENOENT);
    }

    file_descr->file->ioctl(request, arg);
    return KError(0);
}

KErrorOr<fd_t> VFS::socket(int domain, int type, int protocol)
{
    if (domain != AF_LOCAL) {
        return KError(EPROTONOSUPPORT);
    }
    if (type != SOCK_STREAM) {
        return KError(EINVAL);
    }

    return Scheduler::the().current_process().file_descriptions().allocate();
}

KError VFS::bind(fd_t sockfd, const String& path)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(sockfd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    auto relation = resolve_relation(path);
    if (!relation) {
        return relation.error();
    }

    auto file_dentry = relation.result().file;
    if (!file_dentry) {
        return KError(ENOENT);
    }

    auto directory_dentry = relation.result().directory;
    LocalSocket* socket = nullptr;
    const auto& endpoint = file_dentry->name();

    if (!file_dentry->inode()) {
        auto inode = directory_dentry->inode()->create(endpoint, FileType::Socket, 1);
        file_dentry->set_inode(inode);
    }

    socket = LocalSocket::bind_socket(endpoint);
    file_descr->file = socket;

    return KError(0);
}

KError VFS::connect(fd_t sockfd, const String& path)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(sockfd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    auto relation = resolve_relation(path);
    if (!relation) {
        return relation.error();
    }

    auto file_dentry = relation.result().file;
    if (!file_dentry) {
        return KError(ENOENT);
    }

    // TODO: check if the file has the Socket type

    const auto& endpoint = file_dentry->name();
    file_descr->file = LocalSocket::get_socket(endpoint);
    if (!file_descr->file) {
        return KError(ENOTCONN);
    }
    return KError(0);
}

KErrorOr<Relation> VFS::resolve_relation(const String& path)
{
    if (!path.size() || path[0] != '/') {
        return KError(ENOTDIR);
    }

    if (path.size() == 1) {
        Relation rel;
        rel.directory = nullptr;
        rel.file = m_root_dentry;
        return rel;
    }

    Vector<String> splited_path = path.split("/");
    Dentry* parent = nullptr;
    Dentry* cur = m_root_dentry;

    for (size_t i = 1; i < splited_path.size(); i++) {
        Dentry* dentry = cur->lookup(splited_path[i]);

        if (!dentry) {
            return KError(ENOENT);
        }

        if (!dentry->inode()) {
            if (i == splited_path.size() - 1) {
                // at least, we found a parent
                Relation rel = {};
                rel.directory = cur;
                rel.file = dentry;
                return rel;
            }
            return KError(ENOENT);
        }

        parent = cur;
        cur = dentry;
    }

    // we found a parent and a file
    Relation rel;
    rel.directory = parent;
    rel.file = cur;

    return rel;
}

bool VFS::can_read(fd_t fd)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    if (!file_descr->file) {
        return false;
    }

    return file_descr->file->can_read(*file_descr);
}

KError VFS::select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout)
{
    // TODO: select works correctly only for sockets
    if (readfds) {
        FD_ZERO(readfds);
    }
    if (writefds) {
        FD_ZERO(writefds);
    }
    if (execfds) {
        FD_ZERO(execfds);
    }

    for (size_t fd = 0; fd < nfds; fd++) {
        if (readfds) {
            if (can_read(fd)) {
                FD_SET(readfds, fd);
            }
        }
    }

    return KError(0);
}

KErrorOr<size_t> VFS::getdents(fd_t fd, linux_dirent* dirp, size_t size)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    if (!file_descr->file) {
        return KError(ENOENT);
    }

    memset(dirp, 0, size);
    return file_descr->file->getdents(dirp, size);
}

KErrorOr<String> VFS::ptsname(fd_t fd)
{
    auto file_descr_result = Scheduler::the().current_process().file_descriptions().lookup(fd);
    if (!file_descr_result) {
        return KError(EBADF);
    }
    auto file_descr = file_descr_result.result();

    if (!file_descr->file) {
        return KError(ENOENT);
    }

    auto pty_master = static_cast<PTYMaster*>(file_descr->file);
    return String(pty_master->slave()->name());
}

}