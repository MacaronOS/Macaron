#pragma once

#include "PL111Registers.hpp"
#include <Devices/Drivers/Generic/Framebuffer.hpp>

namespace Kernel::Devices {

class LCDPL111 : public Framebuffer<LCDPL111> {
public:
    REGISTER_DRIVER(LCDPL111);

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Framebuffer
    virtual String driver_name() override
    {
        return "PL111";
    }

    virtual String driver_info() override
    {
        return "MacaronOS aarch32 PrimeCell Color LCD Controller (PL111) framebuffer driver";
    }

    // ^Framebuffer
    void map_framebuffer(void* addr, size_t size);
    void swap_buffers();

private:
    uintptr_t m_framebuffer {};
    size_t m_framebuffer_length {};
    bool m_buffer_swapper { true };
    volatile PL111Registers* m_registers;
};

}