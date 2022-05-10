#pragma once

#include <FileSystem/Base/File.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/HashMap.hpp>
#include <Macaronlib/Ringbuffer.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Net {

using namespace FileSystem;

class LocalSocket : public File {
public:
    LocalSocket() = default;

    virtual bool can_read(FileDescription& fd) override
    {
        return m_buffer.space_to_read_from(fd.offset);
    }

    virtual void read(void* buffer, size_t size, FileDescription& fd) override
    {
        fd.offset += m_buffer.read_from((uint8_t*)buffer, fd.offset, size);
    }

    virtual bool can_write(FileDescription&) override
    {
        return true;
    }

    virtual void write(void* buffer, size_t size, FileDescription& fd) override
    {
        m_buffer.write_force((uint8_t*)buffer, size);
    }

    static LocalSocket* get_socket(const String& endpoint)
    {
        return s_endpoint_to_socket[endpoint];
    }

    static LocalSocket* bind_socket(const String& endpoint)
    {
        auto socket = new LocalSocket;
        s_endpoint_to_socket[endpoint] = socket;
        return socket;
    }

private:
    static HashMap<String, LocalSocket*> s_endpoint_to_socket;
    Ringbuffer<16 * 1024> m_buffer {};
};

}