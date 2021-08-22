#pragma once

#include <Drivers/Base/Driver.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

#include <Wisterialib/Function.hpp>
#include <Wisterialib/Vector.hpp>

namespace Kernel::Drivers {

enum class Key {
    Undefined = 0,
    Escape,
    Num_1,
    Num_2,
    Num_3,
    Num_4,
    Num_5,
    Num_6,
    Num_7,
    Num_8,
    Num_9,
    Num_0,
    Minus,
    Equal,
    Backspace,
    Tab,
    Q,
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P,
    LeftBracket,
    RightBracket,
    Enter,
    LeftCtrl,
    A,
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    Semicolon,
    Quote,
    BackTick,
    LeftShift,
    Backslash,
    Z,
    X,
    C,
    V,
    B,
    N,
    M,
    Comma,
    Dot,
    Slash,
    RightShift,
    Asterisk,
    RightAlt,
    Space,
    Capslock,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    NumLock,
    ScrollLock,
    KPNum7,
    KPNum8,
    KPNum9,
    KPMinus,
    KPNum4,
    KPNum5,
    KPNum6,
    KPPlus,
    KPNum1,
    KPNum2,
    KPNum3,
    KPNum0,
    KPDot,
    F11 = 0x57,
    F12,
};

struct KeyboardEvent {
    Key key;
    bool pressed;
};

class Keyboard : public Driver, InterruptHandler {
public:
    Keyboard()
        : Driver(DriverEntity::Keyboard)
        , InterruptHandler(0x21)
    {
    }

    bool install() override;
    void handle_interrupt(Trapframe* tf) override;

    inline void register_callback(const Function<void(KeyboardEvent&)>& callback) { m_callbacks.push_back(callback); }
    inline KeyboardEvent last_keybord_event() const { return m_last_keybord_event; }
    inline void discard_last_keyboard_event() { m_last_keybord_event = { Key::Undefined }; }

private:
    KeyboardEvent m_last_keybord_event { Key::Undefined, false };
    uint8_t data_port { 0x60 };
    uint8_t command_port { 0x64 };
    Vector<Function<void(KeyboardEvent&)>> m_callbacks {};
};

}
