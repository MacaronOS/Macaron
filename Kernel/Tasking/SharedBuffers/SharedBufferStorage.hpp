#pragma once

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/ObjectPool.hpp>

namespace Kernel {

class SharedBufferStorage {
    static constexpr uint32_t MAX_BUFFERS = 100;

    struct SharedBuffer {
        size_t frame;
        size_t pages;
    };

public:
    static SharedBufferStorage& the()
    {
        static SharedBufferStorage the {};
        return the;
    }

    CreateBufferResult create_buffer(uint32_t size);
    uint32_t get_buffer(uint32_t id);

private:
    ObjectPool<SharedBuffer, MAX_BUFFERS> m_free_buffers {};
};

}