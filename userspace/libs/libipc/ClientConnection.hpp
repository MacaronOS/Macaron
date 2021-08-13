#pragma once

#include "IPCHeader.hpp"

#include <wisterialib/String.hpp>
#include <wisterialib/Vector.hpp>

namespace IPC {

class ClientConnection {
public:
    ClientConnection(const String& endpoint);
    inline bool alive() const { return m_alive; }
    void pump();
    void send_data(void* data, size_t bytes);
    inline Vector<Vector<unsigned char>> take_over_messages() { return move(m_message_queue); }

protected:
    void initialize_connection();

protected:
    int m_pid;
    int m_socket_fd;
    int m_server_pid;
    bool m_alive {};
    Vector<Vector<unsigned char>> m_message_queue {};

private:
    IPCHeader m_last_ipch;
    bool m_last_ipch_nead_read {};
    bool read_by_header(const IPCHeader& ipch);
};

}