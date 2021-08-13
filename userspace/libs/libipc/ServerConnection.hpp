#pragma once

#include "IPCHeader.hpp"

#include <wisterialib/String.hpp>
#include <wisterialib/extras.hpp>

namespace IPC {

struct ServerMessage {
    int pid_from;
    Vector<unsigned char> message;
};

class ServerConnection {
public:
    ServerConnection(const String& endpoint);
    void pump();
    void send_data(void* data, size_t size, int pid_to);
    inline Vector<ServerMessage> take_over_messages() { return move(m_message_queue); }

protected:
    int m_pid;
    int m_socket_fd;
    bool m_alive {};
    Vector<ServerMessage> m_message_queue {};

private:
    IPCHeader m_last_ipch;
    bool m_last_ipch_nead_read {};
    bool read_by_header(const IPCHeader& ipch);
};

}