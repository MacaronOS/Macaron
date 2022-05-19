#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Hardware/Trapframe.hpp>
#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::Devices {

template <typename Implementation>
class RealTimeClock : public CharacterDeviceDriver<RealTimeClock<Implementation>> {
public:
    RealTimeClock()
        : CharacterDeviceDriver<RealTimeClock<Implementation>>(10, 135)
    {
    }

    // ^DeviceDriver
    DriverInstallationResult try_install()
    {
        return static_cast<Implementation*>(this)->try_install();
    }

    // ^File
    virtual bool can_read(FileDescription&) override
    {
        return true;
    }
    virtual void read(void* buffer, size_t size, FileDescription&) override
    {
        static_cast<Implementation*>(this)->load_rtc_time(reinterpret_cast<rtc_time*>(buffer));
    }
    virtual void ioctl(uint32_t request, void* arg) override
    {
        switch (request) {
        case RTC_RD_TIME:
            static_cast<Implementation*>(this)->load_rtc_time(reinterpret_cast<rtc_time*>(arg));
            break;
        }
    }
};

}