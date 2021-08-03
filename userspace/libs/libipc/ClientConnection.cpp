#include "ClientConnection.hpp"
#include <libc/syscalls.hpp>
#include <wisterialib/posix/defines.hpp>
#include <wisterialib/extras.hpp>

namespace IPC {

struct MessageHeader {
    int pid_from;
    int pid_to;
    int size;
};

ClientConnection::ClientConnection(const String& endpoint) 
: m_pid(getpid())
{
    m_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 1);
    
    auto c_endpoint = endpoint.cstr();
    int result = -1;

    for (int attempt = 0; attempt < 200; attempt++) {
        result = connect(m_socket_fd, c_endpoint);
        if (result >= 0) {
            break;
        }
        sched_yield();
    }

    if (result < 0) {
        return;
    }

    initialize_connection();

    m_alive = true;
}

void ClientConnection::pump()
{
    MessageHeader mh;
    while (read(m_socket_fd, &mh, sizeof(MessageHeader)) > 0) {
        // skip messages that are not directed to us
        if (mh.pid_to != m_pid) {
            lseek(m_socket_fd, mh.size, SEEK_CUR);
            continue;
        }

        auto memory = Vector<char>(mh.size);
        read(m_socket_fd, memory.data(), mh.size);
        m_message_queue.push_back(move(memory));
    }
}

void ClientConnection::send_data(void* data, size_t bytes)
{
    MessageHeader mh;
    mh.pid_from = m_pid;
    mh.pid_to = m_server_pid;
    mh.size = bytes;

    write(m_socket_fd, &mh, sizeof(MessageHeader));
    write(m_socket_fd, data, bytes);
}

void ClientConnection::initialize_connection()
{
    MessageHeader mh;
    mh.pid_from = m_pid;
    mh.pid_to = -1;
    mh.size = 0;

    write(m_socket_fd, &mh, sizeof(MessageHeader));

    do {
        read(m_socket_fd, &mh, sizeof(MessageHeader));
    } while (mh.pid_to != m_pid);

    m_server_pid = mh.pid_from;
}

}