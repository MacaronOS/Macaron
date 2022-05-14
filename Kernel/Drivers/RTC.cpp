#include "RTC.hpp"
#include <Hardware/x86/Port.hpp>

namespace Kernel::Drivers {

enum {
    cmos_address = 0x70,
    cmos_data = 0x71
};

uint8_t get_RTC_register(int reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}

uint8_t BCD_to_binary(uint8_t num)
{
    return (num & 0x0F) + ((num >> 4) * 10);
}

RtcStruct RTC::get_time()
{
    RtcStruct rtcstruct;
    rtcstruct.second = BCD_to_binary(get_RTC_register(0x00));
    rtcstruct.minute = BCD_to_binary(get_RTC_register(0x02));
    rtcstruct.hour = BCD_to_binary(get_RTC_register(0x04));
    rtcstruct.day = BCD_to_binary(get_RTC_register(0x07));
    rtcstruct.month = BCD_to_binary(get_RTC_register(0x08));
    rtcstruct.year = BCD_to_binary(get_RTC_register(0x09));
    rtcstruct.century = BCD_to_binary(get_RTC_register(0x32));
    return rtcstruct;
}

}
