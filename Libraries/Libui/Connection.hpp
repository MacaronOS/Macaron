#pragma once

#include "Events.hpp"
#include "WSProtocols/ClientConnection.hpp"

namespace UI {

using namespace Protocols;

class Connection : public ClientConnection {
public:
    Connection(const String& endpoint, ClientMessageReciever& reciever)
        : ClientConnection(endpoint, reciever)
    {
        EventLoop::the().register_fd_for_select([this] {
            process_messages();
        },
            m_socket_fd);
    }
};

}