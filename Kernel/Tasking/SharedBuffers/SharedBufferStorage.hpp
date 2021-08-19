#pragma once
#include <Wisterialib/Singleton.hpp>
#include <Wisterialib/Vector.hpp>
#include <Wisterialib/ObjectPool.hpp>
#include <Wisterialib/StaticStack.hpp>
#include <Wisterialib/posix/shared.hpp>
#include <Memory/Region.hpp>

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