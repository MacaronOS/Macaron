#pragma once

#include <FileSystem/Base/File.hpp>

namespace Kernel::Devices {

using namespace FileSystem;

enum class DeviceType {
    Block,
    Char,
};

class Device : public File {
public:
    Device(uint32_t major, uint32_t minor, DeviceType type)
        : m_major(major)
        , m_minor(minor)
        , m_type(type)
    {
    }

    uint32_t major() const { return m_major; }
    uint32_t minor() const { return m_minor; }
    DeviceType type() const { return m_type; }

private:
    uint32_t m_major;
    uint32_t m_minor;
    DeviceType m_type;
};

class CharacterDevice : public Device {
public:
    CharacterDevice(uint32_t major, uint32_t minor)
        : Device(major, minor, DeviceType::Char)
    {
    }
};

class BlockDevice : public Device {
public:
    BlockDevice(uint32_t major, uint32_t minor)
        : Device(major, minor, DeviceType::Block)
    {
    }

    virtual size_t block_size() = 0;
    virtual bool read_blocks(size_t block, size_t block_count, void* buffer) = 0;
    virtual bool write_blocks(size_t block, size_t block_count, void* buffer) = 0;
};

}