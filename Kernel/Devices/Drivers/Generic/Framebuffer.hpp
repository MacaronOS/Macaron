#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Macaronlib/ABI/Syscalls.hpp>

namespace Kernel::Devices {

template <typename Implementation>
class Framebuffer : public CharacterDeviceDriver<Framebuffer<Implementation>> {
public:
    Framebuffer()
        : CharacterDeviceDriver<Framebuffer<Implementation>>(29, 0)
    {
    }

    // ^DeviceDriver
    DriverInstallationResult try_install()
    {
        return static_cast<Implementation*>(this)->try_install();
    }

    // ^File
    virtual void mmap(void* addr, size_t size) override
    {
        return static_cast<Implementation*>(this)->map_framebuffer(addr, size);
    }
    virtual void ioctl(uint32_t request, void* arg) override
    {
        switch (request) {
        case BGA_SWAP_BUFFERS:
            static_cast<Implementation*>(this)->swap_buffers();
            break;
        }
    }
};

}