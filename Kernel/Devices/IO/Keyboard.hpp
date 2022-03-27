#pragma once

#include <Devices/Device.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

#include <Macaronlib/ABI/Keyboard.hpp>
#include <Macaronlib/Function.hpp>
#include <Macaronlib/Ringbuffer.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::Devices {

class Keyboard : public Device, InterruptHandler {
public:
    Keyboard()
        : Device(11, 0, DeviceType::Char)
        , InterruptHandler(0x21)
    {
    }

    // ^Device
    virtual bool install() override;
    virtual bool can_read(FileDescription&) override;
    virtual void read(void* buffer, size_t size, FileDescription& fd) override;

    // ^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

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
