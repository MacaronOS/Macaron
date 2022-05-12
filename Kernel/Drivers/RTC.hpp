#pragma once
#include <Macaronlib/Common.hpp>

namespace Kernel::Drivers {
struct RtcStruct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};

class RTC {
public:
    static RTC& the()
    {
        static RTC the;
        return the;
    }

    RtcStruct get_time();
};
}
