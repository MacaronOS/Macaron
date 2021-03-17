#include "Process.hpp"
#include "TaskManager.hpp"

namespace kernel::multitasking {

ProcessStorage::ProcessStorage()
{
    for (int i = MAX_PROCESSES_ALLOWED - 1; i >= 0; i--) {
        m_free_ids.push(i);
    }
}

Process* ProcessStorage::allocate_process()
{
    if (!m_free_ids.size()) {
        return nullptr;
    }
    auto free_id = m_free_ids.top_and_pop();
    m_process_pool[free_id].id = free_id;
    return &m_process_pool[free_id];
}

void ProcessStorage::free_process(pid_t id)
{
    m_process_pool[id].~Process();
    m_free_ids.push(id);
}

Process& ProcessStorage::operator[](pid_t pid)
{
    return m_process_pool[pid];
}

}