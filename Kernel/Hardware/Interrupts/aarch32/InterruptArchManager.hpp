#pragma once

#include <Devices/Drivers/aarch32/InterruptController/InterruptController.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

namespace Kernel::Hardware {

using namespace Devices;

class InterruptArchManager {
public:
    static auto& the()
    {
        static InterruptArchManager the;
        return the;
    }

    inline void set_interrupt_controller(InterruptController* controller) { m_controller = controller; }

    void setup_interrupts();
    void handle_interrupt(void* trapframe);

private:
    InterruptController* m_controller;
};

class InterruptArchHandler : public InterruptHandler {
    friend class InterruptArchManager;

public:
    InterruptArchHandler(uint8_t interrupt_number, InterruptType type = InterruptType::LevelSensitive)
        : InterruptHandler(interrupt_number)
        , m_type(type)
    {
    }

private:
    InterruptType type() const { return m_type; }

private:
    InterruptType m_type;
};

}

using namespace Kernel::Hardware;