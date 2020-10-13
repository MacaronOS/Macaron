#pragma once
#include "File.hpp"
#include "fs.hpp"
#include "../algo/Array.hpp"
#include "../algo/StaticStack.hpp"
#include "../types.hpp"

namespace kernel::fs {
using algorithms::Array;
using algorithms::StaticStack;

constexpr size_t MAX_FD = 255;

class VFS : public FS {
public:
    VFS();
    ~VFS() = default;

    File& root() override { return *m_root; }
    FileStorage& file_storage() { return m_file_storage; }

    void mount(File& dir, File& appended_dir, const String& appended_dir_name);

    uint32_t read(File& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(File& file, uint32_t offset, uint32_t size, void* buffer) override;

    File* finddir(File& directory, const String& filename) override;
    Vector<String> listdir(File& directory) override;

    File& create(File& directory, const String& name, FileType type, file_permissions_t perms) override;
    bool erase(File& directory, const File& file) override;

private:
    FileStorage m_file_storage {};
    File* m_root { nullptr };
};

}