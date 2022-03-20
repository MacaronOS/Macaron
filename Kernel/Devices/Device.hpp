#pragma once

#include <Filesystem/Base/FileDescriptor.hpp>

namespace Kernel::Devices {

enum class DeviceType {
    Block,
    Character,
};

class Device {
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

    virtual void open(FS::FileDescriptor& fd, FS::VNode& vnode) { fd.set_file(vnode); }
    virtual uint32_t write(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual uint32_t read(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual bool can_read(uint32_t offset) { return false; }
    virtual bool mmap(uint32_t addr, uint32_t size) { return false; };
    virtual bool ioctl(uint32_t request) { return false; };

    virtual bool install() { return true; }
    virtual bool uninstall() { return true; }

private:
    uint32_t m_major;
    uint32_t m_minor;
    DeviceType m_type;
};

}