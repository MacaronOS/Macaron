#pragma once

#include <wisterialib/Vector.hpp>
#include <wisterialib/common.hpp>

#include "../hardware/InterruptManager.hpp"
#include "base/Driver.hpp"
#include "../monitor.hpp"
#include "../hardware/port.hpp"
#include "base/DriverEntity.hpp"

namespace kernel::drivers {

struct TickReciever {
    virtual void on_tick(trapframe_t* tf);
};

class PIT : public Driver, InterruptHandler {
public:
    static constexpr uint32_t frequency = 150;

    PIT()
        : Driver(DriverEntity::PIT)
        , InterruptHandler(32)
    {
    }

    bool install() override; // driver
    void handle_interrupt(trapframe_t* tf) override; // handler

    void register_tick_reciever(TickReciever* reciever);

private:
    Vector<TickReciever*> m_tick_recievers {};
};
}