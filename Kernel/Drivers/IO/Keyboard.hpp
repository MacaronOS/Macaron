#pragma once

#include <Drivers/Base/CharacterDeviceDriver.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

#include <Macaronlib/ABI/Keyboard.hpp>
#include <Macaronlib/Function.hpp>
#include <Macaronlib/Ringbuffer.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::Drivers {

class Keyboard : public CharacterDeviceDriver, InterruptHandler {
public:
    Keyboard()
        : CharacterDeviceDriver(DriverEntity::Keyboard, "kbd")
        , InterruptHandler(0x21)
    {
    }

    bool install() override;
    void handle_interrupt(Trapframe* tf) override;

    uint32_t read(uint32_t offset, uint32_t size, void* buffer) override;
    bool can_read(uint32_t offset) override;

    inline void register_callback(const Function<void(KeyboardPacket&)>& callback) { m_callbacks.push_back(callback); }
    inline KeyboardPacket last_keybord_event() const { return m_last_keybord_event; }
    inline void discard_last_keyboard_event() { m_last_keybord_event = { Key::Undefined }; }

private:
    KeyboardPacket m_last_keybord_event { Key::Undefined, false };
    uint8_t data_port { 0x60 };
    uint8_t command_port { 0x64 };
    Vector<Function<void(KeyboardPacket&)>> m_callbacks {};
    Ringbuffer<1024> m_buffer {};
};

}
