#include "ServerConnection.hpp"
#include <libc/syscalls.hpp>
#include <wisterialib/posix/defines.hpp>
#include <wisterialib/extras.hpp>

namespace IPC {

struct MessageHeader {
    int pid_from;
    int pid_to;
    int size;
};

ServerConnection::ServerConnection(const String& endpoint) 
: m_pid(getpid())
{
    m_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 1);
    int result = bind(m_socket_fd, endpoint.cstr());
    if (result < 0) {
        return;
    }
    m_alive = true;
}

void ServerConnection::pump()
{
    MessageHeader mh;
    while (read(m_socket_fd, &mh, sizeof(MessageHeader)) > 0) {
        // client attempts to initialize a coonection
        if (mh.pid_to == -1) {
            MessageHeader mhr;
            mhr.pid_from = m_pid;
            mhr.pid_to = mh.pid_from;
            write(m_socket_fd, &mhr, sizeof(MessageHeader));
            continue;
        }
        
        // skip messages that are not directed to us
        if (mh.pid_to != m_pid) {
            lseek(m_socket_fd, mh.size, SEEK_CUR);
            continue;
        }

        auto memory = Vector<char>(mh.size);
        read(m_socket_fd, memory.data(), mh.size);
        m_message_queue.push_back({ mh.pid_from, move(memory) });
    }
}

};