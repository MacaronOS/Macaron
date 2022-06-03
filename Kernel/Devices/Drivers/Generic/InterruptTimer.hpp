#pragma once

#include <Devices/Drivers/Driver.hpp>
#include <Hardware/Trapframe.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::Devices {

struct InterruptTimerCallback {
    virtual void on_interrupt_timer(Trapframe*) = 0;
};

class InterruptTimer : public Driver {
public:
    void register_callback(InterruptTimerCallback*);
    static constexpr auto frequency() { return 150; }

protected:
    void dispatch_callbacks(Trapframe*);

private:
    Vector<InterruptTimerCallback*> m_callbacks {};
};

}