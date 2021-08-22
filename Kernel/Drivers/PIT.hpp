#pragma once

#include <Wisterialib/Vector.hpp>
#include <Wisterialib/Common.hpp>

#include "../Hardware/Interrupts/InterruptManager.hpp"
#include "Base/Driver.hpp"
#include "../Hardware/Port.hpp"
#include "Base/DriverEntity.hpp"

namespace Kernel::Drivers {

struct TickReciever {
    virtual void on_tick(Trapframe* tf);
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
    void handle_interrupt(Trapframe* tf) override; // handler

    void register_tick_reciever(TickReciever* reciever);

private:
    Vector<TickReciever*> m_tick_recievers {};
};
}