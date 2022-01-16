#include "VFS.hpp"

#include <Filesystem/DevFS/DevFSNode.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>
#include <PTY/PTYMaster.hpp>
#include <PTY/PTYSlave.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Memory.hpp>
#include <Macaronlib/Runtime.hpp>

namespace Kernel::FS {

VFS::VFS()
{
    m_root = new VNode(nullptr, 2);
    m_file_storage.push(m_root);

    // setting all fds as free
    for (int i = FD_ALLOWED - 1; i > 3; i--) {
        m_free_fds.push(i);
    }
}

KErrorOr<fd_t> VFS::open(const String& path, int flags, mode_t mode)
{
    if (!m_free_fds.size()) {
        return KError(ENFILE);
    }

    auto relation = resolve_relation(path);

    if (!relation) {
        return relation.error();
    }

    auto file = relation.result().file;
    auto direcotry = relation.result().directory;

    if (!file) {
        if ((mode & O_CREAT)) {
            file = create(*direcotry, path.split("/").back(), FileType::File, flags);
        } else {
            return KError(ENOENT);
        }
    }

    fd_t free_fd = m_free_fds.top_and_pop();

    auto& file_descr = m_file_descriptors[free_fd];
    file_descr.set_flags(flags);
    file_descr.set_offset(0);
    file->fs()->open(*file, file_descr);

    return free_fd;
}

KError VFS::close(const fd_t fd)
{
    if (fd > 3 && !get_file_descriptor(fd)) {
        return KError(EBADF);
    }
    m_free_fds.push(fd);
    return KError(0);
}

void VFS::mount(VNode& dir, VNode& appended_dir, const String& appended_dir_name)
{
    dir.mount(VNode::Mountpoint(appended_dir_name, &appended_dir));
}

uint32_t VFS::read(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        return file.fs()->read(file, offset, size, buffer);
    }

    return 0;
}

KErrorOr<size_t> VFS::read(fd_t fd, void* buffer, size_t size)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto vnode = file_descr->vnode();
    if (!vnode) {
        return KError(ENOENT);
    }

    auto socket = vnode->socket();
    auto fs = vnode->fs();
    auto offset = file_descr->offset();

    if (socket) {
        if (!socket->can_read(offset)) {
            //TODO: block here
            return 0;
        }
        size_t read_bytes = socket->read(offset, size, (uint8_t*)buffer);
        file_descr->inc_offset(read_bytes);
        return read_bytes;
    }

    if (fs) {
        size_t read_bytes = fs->read(*vnode, offset, size, buffer);
        file_descr->inc_offset(read_bytes);
        return read_bytes;
    }

    return KError(ENOENT);
}

KErrorOr<size_t> VFS::write(fd_t fd, void* buffer, size_t size)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto vnode = file_descr->vnode();
    if (!vnode) {
        return KError(ENOENT);
    }

    auto socket = vnode->socket();
    auto fs = vnode->fs();
    auto offset = file_descr->offset();

    if (socket) {
        socket->write(size, (uint8_t*)buffer);
        return size;
    }

    if (fs) {
        size_t write_bytes = fs->write(*vnode, offset, size, buffer);
        file_descr->inc_offset(write_bytes);
        return write_bytes;
    }

    return KError(ENOENT);
}

KErrorOr<size_t> VFS::lseek(fd_t fd, size_t offset, int whence)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto socket = file_descr->vnode()->socket();
    if (socket) {
        if (whence == SEEK_SET) {
            file_descr->set_offset(offset);
            return file_descr->offset();
        }

        if (whence == SEEK_CUR) {
            file_descr->inc_offset(offset);
            return file_descr->offset();
        }

        return KError(EINVAL);
    }

    const size_t file_size = file_descr->vnode()->size();

    switch (whence) {
    case SEEK_SET: {
        if (offset >= file_size) {
            return KError(EOVERFLOW);
        }
        file_descr->set_offset(offset);
        break;
    }
    case SEEK_CUR: {
        if (file_descr->offset() + offset >= file_size) {
            return KError(EOVERFLOW);
        }
        file_descr->inc_offset(offset);
        break;
    }
    case SEEK_END: {
        if (file_size - offset < 0) {
            return KError(EOVERFLOW);
        }
        file_descr->set_offset(file_size - offset);
        break;
    }

    default:
        return KError(EINVAL);
    }

    return file_descr->offset();
}

KErrorOr<size_t> VFS::truncate(fd_t fd, size_t size)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    if (file_descr->vnode() && file_descr->vnode()->fs()) {
        size_t new_size = file_descr->vnode()->fs()->truncate(*file_descr->vnode(), size);
        return new_size;
    }

    return KError(ENOENT);
}

KErrorOr<size_t> VFS::file_size(fd_t fd)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }
    if (!file_descr->vnode()) {
        return KError(ENOENT);
    }

    return file_size(*file_descr->vnode());
}

uint32_t VFS::write(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        return file.fs()->write(file, offset, size, buffer);
    }

    return 0;
}

uint32_t VFS::file_size(VNode& file)
{
    if (file.fs()) {
        return file.size();
    }

    return 0;
}

VNode* VFS::finddir(VNode& directory, const String& filename)
{
    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        if (filename == directory.mounted_dirs()[i].name()) {
            return &(directory.mounted_dirs()[i].vnode());
        }
    }
    if (directory.fs()) {
        return directory.fs()->finddir(directory, filename);
    }

    return nullptr;
}

static int cnt = 0;

Vector<String> VFS::listdir(const String& path)
{
    auto r = resolve_path(path);

    if (r) {
        return listdir(*r.result());
    }
    return {};
}

Vector<String> VFS::listdir(VNode& directory)
{
    Vector<String> result {};

    if (directory.fs()) {
        result = directory.fs()->listdir(directory);
    }

    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        auto mounted = directory.mounted_dirs()[i].name();
        result.push_back(move(mounted));
    }

    return result;
}

KError VFS::mmap(fd_t fd, uint32_t addr, uint32_t size)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }
    if (file_descr->vnode() && file_descr->vnode()->fs()) {
        file_descr->vnode()->fs()->mmap(*file_descr->vnode(), addr, size);
        return KError(0);
    }
    return KError(ENOENT);
}

KError VFS::ioctl(fd_t fd, uint32_t request)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }
    if (file_descr->vnode() && file_descr->vnode()->fs()) {
        file_descr->vnode()->fs()->ioctl(*file_descr->vnode(), request);
        return KError(0);
    }
    return KError(ENOENT);
}

KErrorOr<fd_t> VFS::socket(int domain, int type, int protocol)
{
    if (domain != AF_LOCAL) {
        return KError(EPROTONOSUPPORT);
    }
    if (type != SOCK_STREAM) {
        return KError(EINVAL);
    }

    auto free_fd = m_free_fds.top_and_pop();
    m_file_descriptors[free_fd].set_offset(0);
    return free_fd;
}

KError VFS::bind(fd_t sockfd, const String& path)
{
    auto* file_descr = get_file_descriptor(sockfd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto relation = resolve_relation(path);
    if (!relation) {
        return relation.error();
    }

    auto file = relation.result().file;
    if (!file) {
        file = create(*relation.result().directory, path.split("/").back(), FileType::Socket, 1);
    } else {
        // TODO: check if the file has the Socket type
    }

    file->bind_socket();
    file_descr->set_file(file);

    return KError(0);
}

KError VFS::connect(fd_t sockfd, const String& path)
{
    auto* file_descr = get_file_descriptor(sockfd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto relation = resolve_relation(path);
    if (!relation) {
        return relation.error();
    }

    auto file = relation.result().file;

    if (!file) {
        return KError(ENOENT);
    }

    if (!file->socket()) {
        return KError(ENOTCONN);
    }

    file_descr->set_file(file);
    file_descr->set_offset(0);

    return KError(0);
}

VNode* VFS::create(VNode& directory, const String& name, FileType type, file_permissions_t perms)
{
    return directory.fs()->create(directory, name, type, perms);
}

bool VFS::erase(VNode& directory, const VNode& file)
{
    return directory.fs()->erase(directory, file);
}

KErrorOr<VNode*> VFS::resolve_path(const String& path)
{
    if (!path.size() || path[0] != '/') {
        return KError(ENOTDIR);
    }
    if (path == "/") {
        return m_root;
    }

    Vector<String> splited_path = path.split("/");
    VNode* node = &root();

    for (size_t i = 1; i < splited_path.size(); i++) {
        VNode* file = finddir(*node, splited_path[i]);
        if (!file) {
            return KError(ENOENT);
        }
        node = file;
    }
    return node;
}

FileDescriptor* VFS::get_file_descriptor(const fd_t fd)
{
    for (fd_t i : m_free_fds) {
        if (i == fd) {
            return nullptr;
        }
    }
    return &m_file_descriptors[fd];
}

KErrorOr<Relation> VFS::resolve_relation(const String& path)
{
    if (!path.size() || path[0] != '/') {
        return KError(ENOTDIR);
    }

    VNode* node = &root();
    VNode* parent = nullptr;

    if (path == "/") {
        return Relation({ parent, node });
    }

    Vector<String> splited_path = path.split("/");

    for (size_t i = 1; i < splited_path.size(); i++) {

        VNode* file = finddir(*node, splited_path[i]);

        if (!file) {
            if (i == splited_path.size() - 1) {
                // at least, we found a parent
                Relation rel = {};
                rel.directory = node;
                return rel;
            }
            return KError(ENOENT);
        }
        parent = node;
        node = file;
    }

    // we found a parent and a file
    Relation rel;
    rel.directory = parent;
    rel.file = node;

    return rel;
}

bool VFS::can_read(fd_t fd)
{
    auto* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return false;
    }

    auto vnode = file_descr->vnode();
    if (!vnode) {
        return false;
    }

    auto socket = vnode->socket();
    auto fs = vnode->fs();
    auto offset = file_descr->offset();

    if (socket) {
        return socket->can_read(offset);
    }

    if (fs) {
        return fs->can_read(*vnode, offset);
    }

    return false;
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
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    if (file_descr->vnode() && file_descr->vnode()->fs()) {
        memset(dirp, 0, size);
        return file_descr->vnode()->fs()->getdents(*file_descr->vnode(), dirp, size);
    }

    return KError(ENOENT);
}

KErrorOr<String> VFS::ptsname(fd_t fd)
{
    FileDescriptor* file_descr = get_file_descriptor(fd);
    if (!file_descr) {
        return KError(EBADF);
    }

    auto vnode = file_descr->vnode();
    auto devfs_node = static_cast<DevFSNode*>(vnode);
    auto pty_master = static_cast<PTYMaster*>(devfs_node->device());

    return String(pty_master->slave()->name());
}

}