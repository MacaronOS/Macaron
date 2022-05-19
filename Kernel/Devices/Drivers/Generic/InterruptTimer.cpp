#include "InterruptTimer.hpp"

namespace Kernel::Devices {

void InterruptTimer::register_callback(InterruptTimerCallback* callback)
{
    m_callbacks.push_back(callback);
}

void InterruptTimer::dispatch_callbacks(Trapframe* trapframe)
{
    for (auto callback : m_callbacks) {
        callback->on_interrupt_timer(trapframe);
    }
}

}