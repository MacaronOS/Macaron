#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

template <typename Implementation>
class ConsoleDevice : public CharacterDeviceDriver<ConsoleDevice<Implementation>> {
public:
    ConsoleDevice()
        : CharacterDeviceDriver<ConsoleDevice<Implementation>>(5, 1)
    {
    }

    // ^DeviceDriver
    DriverInstallationResult try_install()
    {
        auto result = static_cast<Implementation*>(this)->try_install_console();
        if (result == DriverInstallationResult::Succeeded) {
            Logger::initialize(this);
        }
        return result;
    }

    // ^File
    virtual bool can_read(FileDescription&) override
    {
        return static_cast<Implementation*>(this)->can_receive();
    }
    virtual void read(void* buffer, size_t size, FileDescription& fd) override
    {
        for (size_t i = 0; i < size; i++) {
            ((char*)buffer)[i] = static_cast<Implementation*>(this)->receive_byte();
        }
    }
    virtual bool can_write(FileDescription&) override
    {
        return static_cast<Implementation*>(this)->can_send();
    }
    virtual void write(void* buffer, size_t size, FileDescription&) override
    {
        for (size_t i = 0; i < size; i++) {
            static_cast<Implementation*>(this)->send_byte(((char*)buffer)[i]);
        }
    }
};

}