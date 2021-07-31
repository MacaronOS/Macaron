#pragma once

#include <wisterialib/common.hpp>
#include <wisterialib/posix/shared.hpp>
#include <wisterialib/posix/defines.hpp>
#include <wisterialib/Singleton.hpp>
#include <drivers/PIT.hpp>
#include <errors/KError.hpp>

namespace kernel::time {

using namespace drivers;

class TimeManager : public Singleton<TimeManager>, public TickReciever {
public:
    TimeManager();

    void on_tick(trapframe_t* tf) override;

    KErrorOr<timespec> get_time(int clock_id);

private:
    size_t seconds_since_boot {};
    size_t miliseconds_since_boot {};
};

}