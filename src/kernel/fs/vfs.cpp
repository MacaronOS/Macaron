#include "vfs.hpp"
#include "../algo/extras.hpp"
#include "../assert.hpp"
#include "../monitor.hpp"

namespace kernel::fs {

VFS::VFS()
{
    m_root = &m_file_storage.get(2, nullptr);

    // setting all fds as free
    for (fd_t i = 0; i < FD_ALLOWED; i++) {
        m_free_fds.push(i);
    }
}

KErrorOr<fd_t> VFS::open(const String& path, int flags)
{
    auto file_or_error = resolve_path(path);

    if (!file_or_error) {
        return file_or_error.error();
    }
    if (!m_free_fds.size()) {
        return KError(ENFILE);
    }
    fd_t free_fd = m_free_fds.top_and_pop();
    file_or_error.result()->inc_ref_count();

    m_file_descriptors[free_fd].set_flags(flags);
    m_file_descriptors[free_fd].set_offset(0);
    m_file_descriptors[free_fd].set_file(file_or_error.result());

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

uint32_t VFS::write(File& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        return file.fs()->write(file, offset, size, buffer);
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

Vector<String> VFS::listdir(File& directory)
{
    Vector<String> result;

    if (directory.fs()) {
        result = directory.fs()->listdir(directory);
    }

    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        result.push_back(directory.mounted_dirs()[i].name());
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

}