#include "TimeManager.hpp"
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>

namespace Kernel::Time {

template <>
TimeManager* Singleton<TimeManager>::s_t = nullptr;
template <>
bool Singleton<TimeManager>::s_initialized = false;

TimeManager::TimeManager()
{
    auto pit = reinterpret_cast<PIT*>(Drivers::DriverManager::the().get_driver(DriverEntity::PIT));
    pit->register_tick_reciever(this);
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