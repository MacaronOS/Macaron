#pragma once
#include "StaticStack.hpp"
#include "Vector.hpp"
#include "Common.hpp"

template <typename T, size_t size>
class ObjectPool {
public:
    ObjectPool()
    {
        // TODO: support custom allocators, so kernel will be able to allocate this space more effectively
        m_objects = (T*)malloc(sizeof(T) * size);
        for (int id = size - 1; id >= 0; id--) {
            m_free_ids.push(id);
        }
    }

    ~ObjectPool()
    {
        for (size_t obj_i = 0; obj_i < size; obj_i++) {
            m_objects[obj_i].~T();
        }
        free(m_objects);
        m_objects = nullptr;
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    struct AllocateResult {
        size_t id {};
        T* object {};
    };

    template <typename... Types>
    AllocateResult allocate(Types... args)
    {
        if (!m_free_ids.size()) {
            return {};
        }
        auto free_id = m_free_ids.top_and_pop();
        new (&m_objects[free_id]) T(args...);
        return {
            .id = free_id,
            .object = &m_objects[free_id],
        };
    }

    void deallocate(const size_t id)
    {
        m_objects[id].~T();
        m_free_ids.push(id);
    }

    T* get(size_t id)
    {
        return &m_objects[id];
    }

private:
    T* m_objects;
    StaticStack<size_t, size> m_free_ids {};
};