#pragma once
#include "../algo/Deque.hpp"
#include "../algo/StaticStack.hpp"
#include "Thread.hpp"

namespace kernel::multitasking {

typedef uint8_t pid_t;

constexpr pid_t MAX_PROCESSES_ALLOWED = 20;

enum class ProcessState {
    Running,
};

struct Process {
    uint32_t id;
    ProcessState state;
    uint32_t page_dir_phys;
    uint32_t thread_count;

    Deque<Thread*> m_threads {};
};

class ProcessStorage {
public:
    ProcessStorage();
    // allocates a process, with ID being set
    Process* allocate_process();
    void free_process(pid_t id);

    Process& operator[](pid_t pid);

    size_t size() const { return MAX_PROCESSES_ALLOWED - m_free_ids.size(); }

private:
    Process m_process_pool[MAX_PROCESSES_ALLOWED] {};
    StaticStack<pid_t, MAX_PROCESSES_ALLOWED> m_free_ids {};
};

}