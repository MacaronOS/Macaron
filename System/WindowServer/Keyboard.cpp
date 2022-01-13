#include "Keyboard.hpp"

#include <Libc/Syscalls.hpp>

void Keyboard::pump()
{
    static KeyboardPacket packets[32];
    int bytes = read(m_fd, packets, sizeof(packets));
    for (size_t i = 0; i < bytes / sizeof(KeyboardPacket); i++) {
        m_packets.push_back(packets[i]);
    }
}