#pragma once

#include "../algo/Vector.hpp"
#include "Dirver.hpp"
#include "monitor.hpp"
#include "port.hpp"
#include "types.hpp"
#include "../hardware/InterruptManager.hpp"

namespace kernel::drivers {

constexpr uint32_t default_frequency = 1000;

class PIT : public Driver, InterruptHandler {
public:
    struct Callback {
        uint32_t ticks;
        void (*callback)();
    };

    PIT(uint32_t frequency = default_frequency)
        : m_frequency(frequency), InterruptHandler(32)
    {
    }

    bool install() override; // driver
    void handle_interrupt(trapframe_t* tf) override; // handler

    uint32_t frequency() const { return m_frequency; }

    void register_callback(Callback callback);

private:
    uint32_t m_frequency {};
    uint32_t m_ticks_passed {};
    algorithms::Vector<Callback> m_callbacks {};
};
}