#pragma once

#include "Events.hpp"
#include <libui/WSProtocols/ServerConnection.hpp>
#include <wisterialib/Function.hpp>

using namespace UI::Protocols;

class Connection : public ServerConnection {
public:
    Connection(const String& endpoint, ServerMessageReciever& reciever)
        : ServerConnection(endpoint, reciever)
    {
        EventLoop::the().register_fd_for_select([this] {
            process_messages();
        },
            m_socket_fd);
    }
};