#include "vfs.hpp"
#include "../algo/extras.hpp"
#include "../monitor.hpp"
#include "../assert.hpp"

namespace kernel::fs {

VFS::VFS()
{
    m_root = &m_file_storage.get(2, nullptr);
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

}