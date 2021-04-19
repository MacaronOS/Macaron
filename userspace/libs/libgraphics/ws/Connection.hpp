#pragma once

#include "WSProtocol.hpp"

namespace WS {

constexpr auto endpoint = "/ext2/ws.socket";
constexpr auto uninitialized_connection = -1;

class ClientConnection {
public:
    ClientConnection();
    
    template <typename Request, typename Response>
    Response send_sync_request(Request request)
    {
        auto response = send_sync_message(request.message());
        return Response(response);
    }

private:
    void initialize_connection();
    WSProtocol send_sync_message(WSProtocol& message);

private:
    bool m_alive { false };
    int m_socket_fd;

    int m_local_pid { getpid() };
    int m_server_pid { uninitialized_connection };
};

class ServerConnection {
public:
    ServerConnection();

    bool has_requests();
    WSProtocol recieve_message();

    template <typename Response>
    void send_response_to(Response response, int pid)
    {
        send_async_message_to(response.message(), pid);
    }

private:
    void send_async_message_to(WSProtocol& message, int pid);

private:
    bool m_alive { false };
    int m_socket_fd;

    int m_local_pid { getpid() };
};

}