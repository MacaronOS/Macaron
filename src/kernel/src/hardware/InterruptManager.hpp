#pragma once
#include <wisterialib/common.hpp>
#include <wisterialib/Singleton.hpp>
#include "trapframe.hpp"
#include "../assert.hpp"

typedef void (*interrupt_handler_function)(trapframe_t*);

class InterruptHandler;

class InterruptManager : public Singleton<InterruptManager> {
public:
    InterruptManager() = default;

    void register_interrupt_handler(InterruptHandler* handler);
    void handle_interrupt(trapframe_t* tf);

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

    virtual void handle_interrupt(trapframe_t* tf) {};
    uint8_t interrupt_number() const { return m_interrupt_number; }

private:
    uint8_t m_interrupt_number;
};