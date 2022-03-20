#pragma once

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Vector.hpp>

#include "../Hardware/Interrupts/InterruptManager.hpp"
#include "../Hardware/Port.hpp"
#include "Base/Driver.hpp"
#include "Base/DriverEntity.hpp"

namespace Kernel::Drivers {

struct TickReciever {
    virtual void on_tick(Trapframe* tf);
};

class PIT : public InterruptHandler {
public:
    static constexpr uint32_t frequency = 150;

    static PIT& the()
    {
        static PIT the;
        return the;
    }

    void initialize();
    void handle_interrupt(Trapframe* tf) override;
    void register_tick_reciever(TickReciever* reciever);

private:
    PIT()
        : InterruptHandler(32)
    {
    }

private:
    Vector<TickReciever*> m_tick_recievers {};
};

}