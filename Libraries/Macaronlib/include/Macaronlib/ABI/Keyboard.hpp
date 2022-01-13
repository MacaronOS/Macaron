#pragma once

#include "../Common.hpp"

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

static char KeyToAsci(Key key)
{
    const static char asc[] = {
        '\0', // undefined symbol
        '\0', // esc
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=',
        '\0', '\0', // backspace, tab
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
        '\0', '\0', // enter, leftctrl
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
        ';', '\'', '`',
        '\0', // left shift
        '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/',
        '\0', // right shift
        '*',
        '\0', // right alt
        ' ',
        '\0', // caps lock
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', // f1-f10
        '\0', '\0', // num lock, scroll lock
        '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '4', '.',
        '\0', '\0', '\0', // undefined
        '\0', '\0', // f11, f12
    };
    return asc[static_cast<uint8_t>(key)];
}

struct KeyboardPacket {
    Key key;
    bool pressed;
};