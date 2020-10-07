#include "vfs.hpp"
#include "../algo/extras.hpp"
#include "../monitor.hpp"

namespace kernel::fs {

VFS::VFS()
{
    m_root = File(FileType::Directory);
    m_root.set_permission(FilePermission::Read);
}

void VFS::mount(File& dir, File& appended_dir)
{
    dir.mount(&appended_dir);
}

uint32_t VFS::read(const File& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        file.fs()->read(file, offset, size, buffer);
    }

    return 0;
}

uint32_t VFS::write(const File& file, uint32_t offset, uint32_t size, void* buffer)
{
    if (file.fs()) {
        file.fs()->write(file, offset, size, buffer);
    }

    return 0;
}

File* VFS::finddir(const File& directory, const String& filename)
{
    if (directory.type() != FileType::Directory) {
        return nullptr;
    }

    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        if (directory.mounted_dirs()[i]->name() == filename) {
            return directory.mounted_dirs()[i];
        }
    }

    if (directory.fs()) {
        return directory.fs()->finddir(directory, filename);
    }

    return nullptr;
}

Vector<File*> VFS::listdir(const File& directory)
{
    Vector<File*> result;
    if (directory.type() != FileType::Directory) {
        return result;
    }

    if (directory.fs()) {
        result = directory.fs()->listdir(directory);
    }

    for (size_t i = 0; i < directory.mounted_dirs().size(); i++) {
        result.push_back(directory.mounted_dirs()[i]);
    }

    return result;
}

File& VFS::create(const File& directory, File& file)
{
    if (directory.type() == FileType::Directory && directory.fs()) {
        return directory.fs()->create(directory, file);
    }

    return file;
}

bool VFS::erase(const File& directory, const File& file)
{
    if (directory.type() == FileType::Directory && directory.fs()) {
        return directory.fs()->erase(directory, file);
    }

    return false;
}

}