#pragma once

#include <Devices/Device.hpp>
#include <Devices/Drivers/Generic/InterruptTimer.hpp>
#include <Libkernel/KError.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Time {

using namespace Devices;

class TimeManager : public InterruptTimerCallback {
public:
    static TimeManager& the()
    {
        static TimeManager the {};
        return the;
    }

    bool initialize();
    void on_interrupt_timer(Trapframe* tf) override;
    KErrorOr<timespec> get_time(int clock_id);

private:
    uint32_t calc_seconds_since_epoch();

private:
    Device* m_real_time_clock {};
    size_t seconds_since_boot {};
    size_t miliseconds_since_boot {};
};

}