#pragma once
#include "../WSProtocol.hpp"

namespace WS {

class Wrapper {
public:
    Wrapper(const WSProtocol& message)
        : m_message(message)
    {
    }

    Wrapper(WSProtocol&& message)
        : m_message(move(message))
    {
    }

    WSProtocol& message() { return m_message; }
    const WSProtocol& message() const { return m_message; }

protected:
    WSProtocol m_message;
};

}