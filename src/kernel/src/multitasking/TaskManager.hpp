#pragma once
#include "../algo/Deque.hpp"
#include "../fs/vfs.hpp"
#include "../memory/vmm.hpp"
#include "../assert.hpp"
#include "../algo/Singleton.hpp"

#include "Process.hpp"
#include "Thread.hpp"

namespace kernel::multitasking {

#define DEFAULT_BSS_SIZE 4096
#define DEFAULT_HEAP_SIZE 4096

class TaskManager : public Singleton<TaskManager> {
public:
    TaskManager()
    {
        // setup the initial kernel process
        m_kernel_process = new Process;
        m_kernel_process->id = m_process_count++;
        m_kernel_process->page_dir_phys = VMM::the().kernel_page_directory();
    }

    Process* kernel_process() { return m_kernel_process; }

    void add_kernel_thread(void (*func)());
    void create_process(const String& filepath);

    void schedule(trapframe_t* tf);

    bool run();

private:
    size_t m_process_count { 0 };

    Process* m_kernel_process;

    algorithms::Deque<Thread*> m_threads {};

    algorithms::DequeIterator<algorithms::Deque<Thread*>> m_cur_thread { m_threads.end() };

};
}
