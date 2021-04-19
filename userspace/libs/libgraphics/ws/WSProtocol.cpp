#include "WSProtocol.hpp"

#include <wisterialib/common.hpp>

#include <libsys/Log.hpp>

namespace WS {

WSProtocol::EncodeBuffer WSProtocol::serialize() const
{
    auto buffer = WSProtocol::EncodeBuffer();

    buffer.push(m_type);

    for (int arg : m_args) {
        buffer.push(arg);
    }

    buffer.push(m_pid_to);
    buffer.push(m_pid_from);

    return buffer;
}

WSProtocol WSProtocol::deserialize(uint8_t* buff)
{
    auto buffer = WSProtocol::DecodeBuffer(buff);

    auto type = buffer.get_type();

    auto arg0 = buffer.get_int();
    auto arg1 = buffer.get_int();
    auto arg2 = buffer.get_int();
    auto arg3 = buffer.get_int();

    auto pid_to = buffer.get_int();
    auto pid_from = buffer.get_int();

    auto message = WSProtocol(type, arg0, arg1, arg2, arg3, pid_to, pid_from);

    return message;
}

}