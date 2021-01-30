#pragma once

#include "../algo/Vector.hpp"
#include "../hardware/InterruptManager.hpp"
#include "Dirver.hpp"
#include "../monitor.hpp"
#include "../hardware/port.hpp"
#include "../types.hpp"
#include "DriverEntity.hpp"

namespace kernel::drivers {

constexpr uint32_t default_frequency = 1000;

class PIT : public Driver, InterruptHandler {
public:
    struct Callback {
        uint32_t ticks;
        void (*callback)(trapframe_t*);
    };

    PIT(uint32_t frequency = default_frequency)
        : m_frequency(frequency)
        , Driver(DriverEntity::PIT)
        , InterruptHandler(32)
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