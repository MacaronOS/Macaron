#pragma once

#include <Hardware/Trapframe.hpp>

#include <Macaronlib/Common.hpp>

class InterruptHandler;

class InterruptManager {
public:
    static InterruptManager& the()
    {
        static InterruptManager the {};
        return the;
    }

    void register_interrupt_handler(InterruptHandler* handler);
    void handle_interrupt(uint8_t interrupt_number, Trapframe* tf);

    template <typename Callback>
    void for_each_registered_interrupt(Callback callback)
    {
        for (size_t i = 0; i < 256; i++) {
            if (m_handlers[i] != nullptr) {
                callback(m_handlers[i]);
            }
        }
    }

private:
    InterruptHandler* m_handlers[256] {};
};

class InterruptHandler {
public:
    InterruptHandler(uint8_t interrupt_number)
        : m_interrupt_number(interrupt_number)
    {
        InterruptManager::the().register_interrupt_handler(this);
    }

    virtual void handle_interrupt(Trapframe* tf) {};
    uint8_t interrupt_number() const { return m_interrupt_number; }

private:
    uint8_t m_interrupt_number;
};