#pragma once

#include <Macaronlib/ABI/Keyboard.hpp>
#include <Macaronlib/Vector.hpp>

class Keyboard {
public:
    Keyboard() = default;
    Keyboard(int fd)
        : m_fd(fd)
    {
    }

    void pump();
    inline Vector<KeyboardPacket> take_over_packets() { return move(m_packets); }

private:
    int m_fd {};
    Vector<KeyboardPacket> m_packets {};
};