#pragma once
#pragma once
#include "Thread.hpp"
#include "../algo/Deque.hpp"

namespace kernel::multitasking {

enum class ProcessState {
    Running,
};

struct Process {
    uint32_t id;
    ProcessState state;
    uint32_t page_dir_phys;
    uint32_t thread_count;

    algorithms::Deque<Thread> m_threads {};
};

}