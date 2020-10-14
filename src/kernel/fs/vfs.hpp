#pragma once
#include "../algo/Array.hpp"
#include "../algo/StaticStack.hpp"
#include "../errors/KError.hpp"
#include "../types.hpp"
#include "File.hpp"
#include "fs.hpp"

namespace kernel::fs {
using algorithms::Array;
using algorithms::StaticStack;

constexpr size_t FD_ALLOWED = 255;
typedef uint8_t fd_t;

class VFS {
public:
    VFS();
    ~VFS() = default;

    File& root() { return *m_root; }
    FileStorage& file_storage() { return m_file_storage; }

    void mount(File& dir, File& appended_dir, const String& appended_dir_name);

    KErrorOr<fd_t> open(const String& path, int flags);
    KError close(const fd_t fd);

    uint32_t read(File& file, uint32_t offset, uint32_t size, void* buffer);
    uint32_t write(File& file, uint32_t offset, uint32_t size, void* buffer);

    KErrorOr<size_t> read(fd_t fd, void* buffer, size_t size);

    File* finddir(File& directory, const String& filename);
    Vector<String> listdir(File& directory);

    File& create(File& directory, const String& name, FileType type, file_permissions_t perms);
    bool erase(File& directory, const File& file);

private:
    KErrorOr<File*> resolve_path(const String& path);
    FileDescriptor* get_file_descriptor(const fd_t fd);

private:
    File* m_root { nullptr };

    // Files
    FileStorage m_file_storage {};

    // File Descriptors
    FileDescriptor m_file_descriptors[FD_ALLOWED] {};
    StaticStack<fd_t, FD_ALLOWED> m_free_fds {};
};

}