#include "ClientConnection.hpp"

#include <Libc/Syscalls.hpp>
#include <Libsystem/Log.hpp>
#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Memory.hpp>
#include <Wisterialib/Runtime.hpp>

namespace IPC {

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
    if (m_last_ipch_nead_read) {
        if (!read_by_header(m_last_ipch)) {
            return;
        }
        m_last_ipch_nead_read = false;
    }

    IPCHeader ipch;
    while (read(m_socket_fd, &ipch, sizeof(IPCHeader)) > 0) {
        // skip messages that are not directed to us
        if (ipch.pid_to != m_pid || ipch.pid_from == m_pid) {
            if (ipch.size != 0) {
                lseek(m_socket_fd, ipch.size, SEEK_CUR);
            }
            continue;
        }

        if (!read_by_header(ipch)) {
            m_last_ipch = ipch;
            m_last_ipch_nead_read = true;
            return;
        }
    }
}

void ClientConnection::send_data(void* data, size_t bytes)
{
    IPCHeader ipch;
    ipch.pid_from = m_pid;
    ipch.pid_to = m_server_pid;
    ipch.size = bytes;

    Vector<uint8_t> d(sizeof(IPCHeader) + bytes);
    memcpy(d.data(), &ipch, sizeof(ipch));
    memcpy(d.data() + sizeof(IPCHeader), data, bytes);
    write(m_socket_fd, d.data(), d.size());
}

void ClientConnection::initialize_connection()
{
    IPCHeader ipch;
    ipch.pid_from = m_pid;
    ipch.pid_to = -1;
    ipch.size = 0;

    write(m_socket_fd, &ipch, sizeof(IPCHeader));

    while (true) {
        int result = read(m_socket_fd, &ipch, sizeof(IPCHeader));
        if (result == 0) {
            sched_yield();
            continue;
        }
        if (result == sizeof(IPCHeader)) {
            if (ipch.pid_to == m_pid) {
                m_server_pid = ipch.pid_from;
                return;
            }
            lseek(m_socket_fd, ipch.size, SEEK_CUR);
        }
    }
}

bool ClientConnection::read_by_header(const IPCHeader& ipch)
{
    auto memory = Vector<unsigned char>(ipch.size);
    int bytes = read(m_socket_fd, memory.data(), ipch.size);
    if (bytes <= 0) {
        return false;
    }
    m_message_queue.push_back({ move(memory) });
    return true;
}

}