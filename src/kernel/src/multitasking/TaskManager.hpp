#pragma once
#include "../algo/Deque.hpp"
#include "../algo/Singleton.hpp"
#include "../assert.hpp"
#include "../fs/vfs.hpp"
#include "../memory/vmm.hpp"

#include "Process.hpp"
#include "Thread.hpp"

namespace kernel::multitasking {

#define DEFAULT_BSS_SIZE 4096
#define DEFAULT_HEAP_SIZE 4096

class TaskManager : public Singleton<TaskManager> {
public:
    TaskManager();

    bool run();

    // syscalls handlers
    void sys_exit_handler(int error_code);

    void create_process(const String& filepath);
    void destroy_current_process();
    void destroy_prcoess(const pid_t process);
    void destroy_thread();

private:
    Process* kernel_process() { return m_kernel_process; }
    void add_kernel_thread(void (*func)());
    void schedule(trapframe_t* tf);

private:
    Process* m_kernel_process;
    ProcessStorage m_process_storage {};
    Deque<Thread*> m_threads {};
    DequeIterator<Deque<Thread*>> m_cur_thread { m_threads.end() };
};

}
