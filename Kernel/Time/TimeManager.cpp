#include "TimeManager.hpp"
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>
#include <Drivers/RTC.hpp>

namespace Kernel::Time {

bool TimeManager::initialize()
{
    PIT::the().register_tick_reciever(this);
    return true;
}

void TimeManager::on_tick(Trapframe* tf)
{
    miliseconds_since_boot += (1000 / PIT::frequency);
    if (miliseconds_since_boot >= 1000) {
        seconds_since_boot++;
        miliseconds_since_boot = 0;
    }
}

uint32_t since_epoch()
{
    RtcStruct rtcstruct = RTC::the().get_time();
    auto y = rtcstruct.century * 100 + rtcstruct.year;
    auto m = rtcstruct.month;
    auto d = rtcstruct.day;
    auto h = rtcstruct.hour;
    auto min = rtcstruct.minute;
    auto s = rtcstruct.second;
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const uint32_t yoe = static_cast<uint32_t>(y - era * 400);
    const uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (era * 146097 + static_cast<int>(doe) - 719468) * 86400 + h * 3600 + min * 60 + s;
}

KErrorOr<timespec> TimeManager::get_time(int clock_id)
{
    if (clock_id == CLOCK_MONOTONIC_COARSE) {
        timespec ts;
        ts.tv_sec = seconds_since_boot;
        ts.tv_nsec = miliseconds_since_boot * 1000000;
        return ts;
    }

    if (clock_id == CLOCK_REALTIME) {
        timespec ts;
        ts.tv_sec = since_epoch();
        ts.tv_nsec = 0;
        return ts;
    }
    return KError(EOVERFLOW);
}

}