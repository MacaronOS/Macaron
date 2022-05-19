#include "TimeManager.hpp"
#include <Devices/DeviceManager.hpp>
#include <Libkernel/Assert.hpp>

namespace Kernel::Time {

bool TimeManager::initialize()
{
    m_real_time_clock = DeviceManager::the().get_device(10, 135, DeviceType::Char);
    auto pit = static_cast<InterruptTimer*>(DeviceManager::the().find_attached_driver_by_name("PIT"));
    if (!pit) {
        ASSERT_PANIC("[TimeManager] Can not find interrupt timer");
    }
    pit->register_callback(this);
    return true;
}

void TimeManager::on_interrupt_timer(Trapframe* tf)
{
    miliseconds_since_boot += (1000 / InterruptTimer::frequency());
    if (miliseconds_since_boot >= 1000) {
        seconds_since_boot++;
        miliseconds_since_boot = 0;
    }
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
        ts.tv_sec = calc_seconds_since_epoch();
        ts.tv_nsec = 0;
        return ts;
    }
    return KError(EOVERFLOW);
}

uint32_t TimeManager::calc_seconds_since_epoch()
{
    if (!m_real_time_clock) {
        return 0;
    }

    // HACK: use ioctl of the RTC device to connect to the driver and ask it about time.
    rtc_time time;
    m_real_time_clock->ioctl(RTC_RD_TIME, &time);

    auto y = time.tm_year;
    auto m = time.tm_mon;
    auto d = time.tm_mday;
    auto h = time.tm_hour;
    auto min = time.tm_min;
    auto s = time.tm_sec;

    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const uint32_t yoe = static_cast<uint32_t>(y - era * 400);
    const uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return (era * 146097 + static_cast<int>(doe) - 719468) * 86400 + h * 3600 + min * 60 + s;
}

}