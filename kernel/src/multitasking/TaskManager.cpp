#include "TaskManager.hpp"

#include <Logger.hpp>

#include <drivers/base/DriverEntity.hpp>
#include <drivers/DriverManager.hpp>
#include <drivers/PIT.hpp>
#include <fs/vfs/vfs.hpp>
#include <hardware/descriptor_tables.hpp>
#include <memory/malloc.hpp>

namespace kernel::multitasking {

extern "C" void return_from_scheduler(trapframe_t* tf);
extern "C" void return_to_the_kernel_handler(KernelContext* kc);
extern "C" void switch_to_user_mode();

template <>
TaskManager* Singleton<TaskManager>::s_t = nullptr;
template <>
bool Singleton<TaskManager>::s_initialized = false;

using namespace memory;
using namespace Logger;

TaskManager::TaskManager()
{
    m_process_storage = new ProcessStorage();
}

bool TaskManager::run()
{
    auto* pit = reinterpret_cast<drivers::PIT*>(drivers::DriverManager::the().get_driver(drivers::DriverEntity::PIT));
    if (pit) {
        m_cur_thread = m_threads.begin();
        pit->register_callback({ drivers::default_frequency, [](trapframe_t* tf) { TaskManager::the().schedule(tf); } });
        switch_to_user_mode();
        STOP();
    }
    return false;
}

void TaskManager::schedule(trapframe_t* tf)
{
    auto next_thread = m_cur_thread;

    while (++next_thread != m_cur_thread) {
        if (next_thread == m_threads.end()) {
            next_thread = m_threads.rend();
            continue;
        }
        auto thread_ptr = *next_thread;
        if (thread_ptr->state() == ThreadState::Terminated) {
            delete thread_ptr;
            next_thread = m_threads.remove(next_thread);
            continue;
        }
        break;
    }

    auto next_thread_ptr = *next_thread;

    // switch to the new thread's tss entry
    set_kernel_stack(next_thread_ptr->kernel_stack_top());

    // swtich to the new process address space
    VMM::the().set_page_directory(next_thread_ptr->m_process->m_pdir_phys);

    next_thread_ptr->m_process->cur_thread = next_thread_ptr;

    m_cur_thread = next_thread;

    return_from_scheduler(next_thread_ptr->trapframe());
}

void TaskManager::create_process(const String& filepath)
{
    auto new_process = m_process_storage->allocate_process();
    if (new_process) {
        new_process->LoadAndPrepare(filepath);
    } else {
        Logger::Log() << "Error: out of processes\n";
    }
}

void TaskManager::sys_exit_handler(int error_code)
{
    Log() << "Handling exit, PID: " << (*m_cur_thread)->m_process->id() << "\n";
    cur_process()->Terminate();
    schedule(nullptr);
}

int TaskManager::sys_fork_handler()
{
    Log() << "Handling fork\n";
    auto forked_process = cur_process()->Fork();
    if (!forked_process) {
        return -1;
    }
    return forked_process->id();
}

int TaskManager::sys_execve_handler(const char* filename, const char* const* argv, const char* const* envp)
{
    // TODO: error handling
    cur_process()->LoadAndPrepare(filename);
    return 1;
}

Thread* TaskManager::cur_thread()
{
    return *m_cur_thread;
}

Process* TaskManager::cur_process()
{
    return cur_thread()->m_process;
}

}