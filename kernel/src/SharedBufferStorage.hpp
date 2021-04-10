#pragma once
#include <wisterialib/Singleton.hpp>
#include <wisterialib/Vector.hpp>
#include <wisterialib/ObjectPool.hpp>
#include <wisterialib/StaticStack.hpp>
#include <wisterialib/posix/shared.hpp>
#include <memory/Region.hpp>

namespace kernel {

class SharedBufferStorage : public Singleton<SharedBufferStorage> {
    static constexpr uint32_t MAX_BUFFERS = 100;
public:
    SharedBufferStorage() = default;

    CreateBufferResult create_buffer(uint32_t size);
    uint32_t get_buffer(uint32_t id);

private:
    ObjectPool<memory::Region, MAX_BUFFERS> m_free_regions {};
};

}