/*
WSProtocol represents a class that's used for communitcation between Window Server and Application.
It provides pid_to and pid_from fields, that's controlled by Connection, Type and 4 arguments, that depends on
the Type. WSProtocol is wraped by wrappers (see ./wrappers folder) for better readability.
*/

#pragma once

#include <wisterialib/Vector.hpp>
#include <wisterialib/common.hpp>

#include <libc/syscalls.hpp>

namespace WS {

class Wrapper;

class WSProtocol {
    friend class Wrapper;

public:
    static constexpr auto arguments = 4;
    enum class Type : uint8_t {
        InitialzieConnectionRequest,
        InitialzieConnectionResponse,

        CreateWindowRequest,
        CreateWindowResponse,
    };

    class EncodeBuffer {
    public:
        EncodeBuffer() = default;

        void push(int val)
        {
            m_buffer.push_back((uint8_t)val);
            m_buffer.push_back((uint8_t)(val >> 8));
            m_buffer.push_back((uint8_t)(val >> 16));
            m_buffer.push_back((uint8_t)(val >> 24));
        }

        void push(WSProtocol::Type type)
        {
            m_buffer.push_back((uint8_t)type);
        }

        uint8_t* data() { return m_buffer.data(); }
        size_t size() const { return m_buffer.size(); }

    private:
        Vector<uint8_t> m_buffer {};
    };

    class DecodeBuffer {
    public:
        DecodeBuffer(uint8_t* data)
            : m_data(data)
        {
        }

        int get_int()
        {
            return (int)m_data[m_offset++] | ((int)m_data[m_offset++]) << 8 | ((int)m_data[m_offset++]) << 16 | ((int)m_data[m_offset++]) << 24;
        }

        Type get_type()
        {
            return (Type)m_data[m_offset++];
        }

    private:
        uint8_t* m_data;
        size_t m_offset;
    };

public:
    WSProtocol(
        WSProtocol::Type type = WSProtocol::Type::InitialzieConnectionRequest,
        int arg1 = 0, int arg2 = 0, int arg3 = 0, int arg4 = 0,
        int pid_to = -1, int pid_from = getpid())

        : m_type(type)
        , m_pid_to(pid_to)
        , m_pid_from(pid_from)
    {
        m_args[0] = arg1;
        m_args[1] = arg2;
        m_args[2] = arg3;
        m_args[3] = arg4;
    }

    static constexpr uint32_t size() { return sizeof(WSProtocol::Type) + 6 * sizeof(int); }

    void set_pid_to(int pid_to) { m_pid_to = pid_to; }
    void set_pid_from(int pid_from) { m_pid_from = pid_from; }

    int pid_to() const { return m_pid_to; }
    int pid_from() const { return m_pid_from; }

    WSProtocol::Type type() const { return m_type; }

    EncodeBuffer serialize() const;
    static WSProtocol deserialize(uint8_t* buffer);

protected:
    int m_pid_to;
    int m_pid_from;

    WSProtocol::Type m_type;

public:
    int m_args[arguments] {};
};

}