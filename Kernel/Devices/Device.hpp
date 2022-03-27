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

    virtual bool install() { return true; }
    virtual bool uninstall() { return true; }

private:
    uint32_t m_major;
    uint32_t m_minor;
    DeviceType m_type;
};

}