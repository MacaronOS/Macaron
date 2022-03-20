#include "TimeManager.hpp"
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>

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

KErrorOr<timespec> TimeManager::get_time(int clock_id)
{
    if (clock_id == CLOCK_MONOTONIC_COARSE) {
        timespec ts;
        ts.tv_sec = seconds_since_boot;
        ts.tv_nsec = miliseconds_since_boot * 1000000;
        return ts;
    }
    return KError(EOVERFLOW);
}

}