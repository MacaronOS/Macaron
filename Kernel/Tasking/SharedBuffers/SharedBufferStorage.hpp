#pragma once

#include <Memory/Region.hpp>

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/ObjectPool.hpp>
#include <Wisterialib/Singleton.hpp>

namespace Kernel {

class SharedBufferStorage : public Singleton<SharedBufferStorage> {
    static constexpr uint32_t MAX_BUFFERS = 100;

public:
    SharedBufferStorage() = default;

    CreateBufferResult create_buffer(uint32_t size);
    uint32_t get_buffer(uint32_t id);

private:
    ObjectPool<Memory::Region, MAX_BUFFERS> m_free_regions {};
};

}