#include "Connection.hpp"
#include <libc/syscalls.hpp>
#include <libsys/syscalls.hpp>
#include <wisterialib/posix/defines.hpp>

#include <libsys/Log.hpp>

namespace WS {

ClientConnection::ClientConnection()
{
    m_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 1);

    int connection_result = -1;

    for (int attempt = 0; attempt < 200; attempt++) {
        connection_result = connect(m_socket_fd, endpoint);
        if (connection_result >= 0) {
            break;
        }
    }

    if (connection_result < 0) {
        return;
    }

    initialize_connection();

    m_alive = true;
}

void ClientConnection::initialize_connection()
{
    auto init_message = WSProtocol();
    auto response = send_sync_message(init_message);
    m_server_pid = response.pid_from();
}

WSProtocol ClientConnection::send_sync_message(WSProtocol& message)
{
    message.set_pid_from(m_local_pid);
    message.set_pid_to(m_server_pid);

    auto buffer = message.serialize();

    write(m_socket_fd, buffer.data(), buffer.size());

    while (true) {
        static uint8_t buff[WSProtocol::size()];
        auto bytes = read(m_socket_fd, &buff, WSProtocol::size());

        if (bytes == WSProtocol::size()) {
            auto response_message = WSProtocol::deserialize(buff);
            if (response_message.pid_to() == m_local_pid) {
                return response_message;
            }
        }
    }
}

ServerConnection::ServerConnection()
{
    m_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 1);
    int bind_result = bind(m_socket_fd, endpoint);
    if (bind_result < 0) {
        return;
    }
    m_alive = true;
}

bool ServerConnection::has_requests()
{
    return can_read(m_socket_fd);
}

WSProtocol ServerConnection::recieve_message()
{
    while (true) {
        static uint8_t buff[WSProtocol::size()];
        auto bytes = read(m_socket_fd, &buff, WSProtocol::size());

        if (bytes == WSProtocol::size()) {
            auto message = WSProtocol::deserialize(buff);

            // special case
            if (message.pid_to() == uninitialized_connection) {
                auto init_conn_resp = WSProtocol();
                send_async_message_to(init_conn_resp, message.pid_from());
                continue;
            }

            return message;
        }
    }
}

void ServerConnection::send_async_message_to(WSProtocol& message, int pid)
{
    message.set_pid_from(m_local_pid);
    message.set_pid_to(pid);

    auto buffer = message.serialize();
    write(m_socket_fd, buffer.data(), buffer.size());
}

}