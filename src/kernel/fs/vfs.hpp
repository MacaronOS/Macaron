#pragma once
#include "File.hpp"
#include "fs.hpp"

namespace kernel::fs {

class VFS : public FS {
public:
    VFS();
    ~VFS() = default;

    File& root() override { return m_root; }

    void mount(File& dir, File& appended_dir);

    uint32_t read(const File& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(const File& file, uint32_t offset, uint32_t size, void* buffer) override;
    File* finddir(const File& directory, const String& filename) override;
    Vector<File*> listdir(const File& directory) override;
    File& create(const File& directory, File& file) override;
    bool erase(const File& directory, const File& file) override;

private:
    File m_root;
};

}