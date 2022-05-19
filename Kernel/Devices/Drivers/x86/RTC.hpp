#pragma once

#include <Devices/Drivers/Generic/RealTimeClock.hpp>

namespace Kernel::Devices {

class RTC : public RealTimeClock<RTC> {
public:
    REGISTER_DRIVER(RTC)

    // ^DevicDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "RTC";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 Real Time Clock driver";
    }

    // ^RealTimeClock
    void load_rtc_time(rtc_time*);
};

}