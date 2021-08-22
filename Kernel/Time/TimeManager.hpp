#pragma once

#include <Drivers/PIT.hpp>
#include <Libkernel/KError.hpp>

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Common.hpp>
#include <Wisterialib/Singleton.hpp>

namespace Kernel::Time {

using namespace Drivers;

class TimeManager : public Singleton<TimeManager>, public TickReciever {
public:
    TimeManager();

    void on_tick(Trapframe* tf) override;

    KErrorOr<timespec> get_time(int clock_id);

private:
    size_t seconds_since_boot {};
    size_t miliseconds_since_boot {};
};

}