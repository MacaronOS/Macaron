#pragma once
#include "MemoryDescription/MemoryDescription.hpp"
#include "Scheduler.hpp"
#include "Thread.hpp"

#include <Filesystem/Base/File.hpp>
#include <Memory/vmm.hpp>

#include <Macaronlib/Memory.hpp>
#include <Macaronlib/StaticStack.hpp>

typedef uint8_t pid_t;

namespace Kernel::Tasking {

using namespace FileSystem;
using namespace Memory;

// TODO: by increasing this value, GCC compilation slows down
constexpr pid_t MAX_PROCESSES_ALLOWED = 20;

class ProcessStorage;
class Scheduler;
class Thread;

class Process {
    friend class ProcessStorage;
    friend class Thread;
    friend class Scheduler;

public:
    inline uint32_t id() const { return m_id; }

public:
    void terminate();
    Process* fork();
    void load(const String& binary);

    FileDescription* file_description(fd_t fd);
    KError free_file_descriptor(fd_t fd);
    KErrorOr<fd_t> allocate_file_descriptor();

    MemoryDescription& memory_description() { return m_memory_description; }

private:
    Process() = default;
    explicit Process(uint32_t id);

private:
    void free_threads_except_one();
    inline void add_thread(Thread* thread) { m_threads.push_back(thread); }

    ProcessStorage* PS() const;
    List<Thread*>& TS() const;

    bool is_file_descriptor_in_use(fd_t fd);

public:
    Scheduler* m_task_manager {};

    uint32_t m_id {};
    Thread* cur_thread {};

    List<Thread*> m_threads {};

    Array<FileDescription, 32> m_file_descriptions {};
    StaticStack<fd_t, 32> m_free_file_descriptors {};

    MemoryDescription m_memory_description {};
};

// TODO: implement object pool

class ProcessStorage {
public:
    ProcessStorage();
    // allocates a process, with ID being set
    Process* allocate_process();
    void free_process(pid_t id);

    Process& operator[](pid_t pid);

    [[nodiscard]] size_t size() const { return MAX_PROCESSES_ALLOWED - m_free_ids.size(); }

private:
    uint8_t m_process_pool[MAX_PROCESSES_ALLOWED * sizeof(Process)] {};
    StaticStack<pid_t, MAX_PROCESSES_ALLOWED> m_free_ids {};
};

}