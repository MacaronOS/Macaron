#pragma once
#include "Wrapper.hpp"
#include "../WSProtocol.hpp"

#include <wisterialib/extras.hpp>

namespace WS {

class CreateWindowRequest : public Wrapper {
public:
    CreateWindowRequest()
        : Wrapper(WSProtocol(WSProtocol::Type::CreateWindowRequest))
    {
    }
};

class CreateWindowResponse : public Wrapper {
public:
    using Wrapper::Wrapper;

    CreateWindowResponse(int buffer_id, int window_id)
        : Wrapper(WSProtocol(WSProtocol::Type::CreateWindowResponse, buffer_id, window_id))
    {
    }

    int buffer_id() const { return m_message.m_args[0]; }
    int window_id() const { return m_message.m_args[1]; }
};

}
