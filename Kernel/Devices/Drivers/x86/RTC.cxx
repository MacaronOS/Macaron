#include "RTC.hpp"
#include <Hardware/x86/Port.hpp>

namespace Kernel::Devices {

RTC rtc;

enum {
    cmos_address = 0x70,
    cmos_data = 0x71
};

static inline uint8_t get_rtc_register(int reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}

static inline uint8_t bcd_to_binary(uint8_t num)
{
    return (num & 0x0F) + ((num >> 4) * 10);
}

DriverInstallationResult RTC::try_install()
{
    return DriverInstallationResult::Succeeded;
}

void RTC::load_rtc_time(rtc_time* time)
{
    time->tm_sec = bcd_to_binary(get_rtc_register(0x00));
    time->tm_min = bcd_to_binary(get_rtc_register(0x02));
    time->tm_hour = bcd_to_binary(get_rtc_register(0x04));
    time->tm_mday = bcd_to_binary(get_rtc_register(0x07));
    time->tm_mon = bcd_to_binary(get_rtc_register(0x08));
    time->tm_year = bcd_to_binary(get_rtc_register(0x09));

    auto century = bcd_to_binary(get_rtc_register(0x32));
    time->tm_year += century * 100;
}

}