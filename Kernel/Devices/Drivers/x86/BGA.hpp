#pragma once

#include <Devices/Drivers/Generic/Framebuffer.hpp>

namespace Kernel::Devices {

class BGA : public Framebuffer<BGA> {
public:
    REGISTER_DRIVER(BGA);

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Framebuffer
    virtual String driver_name() override
    {
        return "BGA";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 Bochs VBE framebuffer driver";
    }

    // ^Framebuffer
    void map_framebuffer(void* addr, size_t size);
    void swap_buffers();

private:
    void* m_framebuffer {};
    size_t m_framebuffer_length {};
    bool m_buffer_swapper { true };
};

}