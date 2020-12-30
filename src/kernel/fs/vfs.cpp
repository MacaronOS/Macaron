#include "vfs.hpp"
#include "../algo/extras.hpp"
#include "../assert.hpp"
#include "../monitor.hpp"
#include "../posix.hpp"

namespace kernel::fs {

template <>
VFS* Singleton<VFS>::s_t = nullptr;
template <>
bool Singleton<VFS>::s_initialized = false;

VFS::VFS()
{
    m_root = &m_file_storage.get(2, nullptr);

    // setting all fds as free
    for (fd_t i = 0; i < FD_ALLOWED; i++) {
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

    if (!file) {
        if ((mode & O_CREAT)) {
            file = &create(*relation.result().directory, path.split("/").back(), FileType::File, flags);
        } else {
            return KError(ENOENT);
        }
    }

    fd_t free_fd = m_free_fds.top_and_pop();
    file->inc_ref_count();

    m_file_descriptors[free_fd].set_flags(flags);
    m_file_descriptors[free_fd].set_offset(0);
    m_file_descriptors[free_fd].set_file(file);

    return free_fd;
}

KError VFS::close(const fd_t fd)
{
    if (!get_file_descriptor(fd)) {
        return KError(EBADF);
    }
    m_free_fds.push(fd);
    return KError(0);
}

void VFS::mount(File& dir, File& appended_dir, const String& appended_dir_name)
{
    dir.mount(Mountpoint(appended_dir_name, &appended_dir));
}

uint32_t VFS::read(File& file, uint32_t offset, uint32_t size, void* buffer)
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
    if (file_descr->file() && file_descr->file()->fs()) {
        size_t read_bytes = file_descr->file()->fs()->read(*file_descr->file(), file_descr->offset(), size, buffer);
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
    if (file_descr->file() && file_descr->file()->fs()) {
        size_t write_bytes = file_descr->file()->fs()->write(*file_descr->file(), file_descr->offset(), size, buffer);
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

    const size_t file_size = file_descr->file()->inode_struct()->size;

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

    if (file_descr->file() && file_descr->file()->fs()) {
        size_t new_size = file_descr->file()->fs()->truncate(*file_descr->file(), size);
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
    if (!file_descr->file()) {
        return KError(ENOENT);
    }

    return file_size(*file_descr->file());
}

uint32_t VFS::write(File& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        return file.fs()->write(file, offset, size, buffer);
    }

    return 0;
}

uint32_t VFS::file_size(File& file)
{
    if (file.fs()) {
        return file.inode_struct()->size;
    }

    return 0;
}

File* VFS::finddir(File& directory, const String& filename)
{
    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        if (filename == directory.mounted_dirs()[i].name()) {
            return &(directory.mounted_dirs()[i].file());
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

Vector<String> VFS::listdir(File& directory)
{
    Vector<String> result;

    if (directory.fs()) {
        result = directory.fs()->listdir(directory);
    }

    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        auto mounted = directory.mounted_dirs()[i].name();
        result.push_back(move(mounted));
    }

    return result;
}

File& VFS::create(File& directory, const String& name, FileType type, file_permissions_t perms)
{
    return directory.fs()->create(directory, name, type, perms);
}

bool VFS::erase(File& directory, const File& file)
{
    return directory.fs()->erase(directory, file);
}

KErrorOr<File*> VFS::resolve_path(const String& path)
{
    if (!path.size() || path[0] != '/') {
        return KError(ENOTDIR);
    }
    if (path == "/") {
        return m_root;
    }

    Vector<String> splited_path = path.split("/");
    File* node = &root();

    for (size_t i = 1; i < splited_path.size(); i++) {
        File* file = finddir(*node, splited_path[i]);
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

    File* node = &root();
    File* parent = nullptr;

    if (path == "/") {
        return Relation({ parent, node });
    }

    Vector<String> splited_path = path.split("/");

    for (size_t i = 1; i < splited_path.size(); i++) {

        File* file = finddir(*node, splited_path[i]);

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

}