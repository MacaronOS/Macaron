#pragma once

#include <wisterialib/String.hpp>
#include <wisterialib/Vector.hpp>

namespace IPC {

class ClientConnection {
public:
    ClientConnection(const String& endpoint);
    void pump();
    void send_data(void* data, size_t bytes);
    inline Vector<Vector<char>> take_over_messages() { return move(m_message_queue); }

private:
    void initialize_connection();

private:
    int m_pid;
    int m_socket_fd;
    int m_server_pid;
    bool m_alive {};
    Vector<Vector<char>> m_message_queue {};
};

}