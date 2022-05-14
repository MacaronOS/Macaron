#pragma once

#include "FileDescriptions.hpp"
#include "Threads/Thread.hpp"
#include <Macaronlib/Common.hpp>
#include <Macaronlib/List.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

class Scheduler;

class Process {
    friend class Scheduler;
    using ProcessPool = ObjectPool<Process, 32>;
    friend ProcessPool;

public:
    inline uint32_t id() { return m_id; }
    inline MemoryDescription& memory_description() { return m_memory_description; }
    inline FileDescriptions& file_descriptions() { return m_file_desciptions; }
    Thread& current_thread();

    KErrorOr<Process*> fork();
    KError exec(const String& bindary);
    void terminate();

    static KErrorOr<Process*> find_process_by_id(int id);

private:
    Process() = default;

    inline void set_current_thread(Thread* thread)
    {
        m_current_thread = thread;
    }

    enum class AllocateUserStack {
        Yes,
        No,
    };
    KErrorOr<Thread*> create_thread(AllocateUserStack);
    void free_memory_except_current_thread_stack();

    static void create_initial_process();

private:
    uint32_t m_id {};
    MemoryDescription m_memory_description {};
    FileDescriptions m_file_desciptions {};
    List<Thread*> m_threads {};
    Thread* m_current_thread {};
    static ProcessPool s_process_pool;
};

}
