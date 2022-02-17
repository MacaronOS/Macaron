#include "Scheduler.hpp"
#include "Signal.hpp"
#include <Drivers/Base/DriverEntity.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Hardware/DescriptorTables/GDT.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Tasking {

extern "C" void return_from_scheduler(Trapframe* tf);
extern "C" void return_to_the_kernel_handler(KernelContext* kc);
extern "C" void switch_to_user_mode();

using namespace Memory;
using namespace Logger;

Scheduler::Scheduler()
{
    m_process_storage = new ProcessStorage();
}

bool Scheduler::run()
{
    auto* pit = reinterpret_cast<Drivers::PIT*>(Drivers::DriverManager::the().get_driver(Drivers::DriverEntity::PIT));
    if (pit) {
        m_running = true;
        m_cur_thread = m_threads.begin();
        pit->register_tick_reciever(this);
        switch_to_user_mode();
        reschedule();
    }
    return false;
}

void Scheduler::reschedule()
{
    auto next_thread = m_cur_thread;
    next_thread++;

    while (next_thread != m_cur_thread) {
        if (next_thread == m_threads.end()) {
            next_thread = m_threads.begin();
            continue;
        }

        auto thread_ptr = *next_thread;
        if (thread_ptr->state() == ThreadState::Terminated) {
            delete thread_ptr;
            next_thread = m_threads.remove(next_thread);
            continue;
        } else {
            break;
        }

        ++next_thread;
    }

    auto next_thread_ptr = *next_thread;

    // switch to the new thread's tss entry
    DescriptorTables::GDT::SetKernelStack(next_thread_ptr->kernel_stack_top());

    // swtich to the new process address space
    VMM::the().set_page_directory(next_thread_ptr->m_process->m_pdir_phys);

    next_thread_ptr->m_process->cur_thread = next_thread_ptr;

    m_cur_thread = next_thread;

    // Dispatch pending signals
    for (int signo = 1; signo < NSIG; signo++) {
        if (next_thread_ptr->signal_is_pending(signo)) {
            next_thread_ptr->signal_remove_pending(signo);
            if (next_thread_ptr->signal_handler(signo)) {
                auto trapframe = next_thread_ptr->trapframe();
                trapframe->push(trapframe->useresp);
                trapframe->push(trapframe->eflags);
                trapframe->push(trapframe->eip);
                trapframe->push(trapframe->eax);
                trapframe->push(trapframe->ecx);
                trapframe->push(trapframe->edx);
                trapframe->push(trapframe->ebx);
                trapframe->push(trapframe->esp);
                trapframe->push(trapframe->ebp);
                trapframe->push(trapframe->esi);
                trapframe->push(trapframe->edi);
                trapframe->push(signo);
                trapframe->eax = (uint32_t)next_thread_ptr->signal_handler(signo);
                trapframe->eip = next_thread_ptr->m_process->m_signal_handler_ip;
            } else {
                auto action = default_action(signo);
                if (action == DefaultAction::Terminate) {
                    next_thread_ptr->m_process->Terminate();
                    reschedule();
                    return;
                }
            }
        }
    }

    return_from_scheduler(next_thread_ptr->trapframe());
}

void Scheduler::create_process(const String& filepath)
{
    auto new_process = m_process_storage->allocate_process();
    if (new_process) {
        new_process->LoadAndPrepare(filepath);
    } else {
        Logger::Log() << "Error: out of processes\n";
    }
}

void Scheduler::sys_exit_handler(int error_code)
{
    Log() << "Handling exit, PID: " << (*m_cur_thread)->m_process->id() << ", error code: " << error_code << "\n";
    cur_process()->Terminate();
    reschedule();
}

int Scheduler::sys_fork_handler()
{
    Log() << "Handling fork\n";
    auto forked_process = cur_process()->Fork();
    if (!forked_process) {
        return -1;
    }
    return forked_process->id();
}

int Scheduler::sys_execve_handler(const char* filename, const char* const* argv, const char* const* envp)
{
    // TODO: error handling
    cur_process()->LoadAndPrepare(filename);
    return 1;
}

Thread* Scheduler::cur_thread()
{
    return *m_cur_thread;
}

Process* Scheduler::cur_process()
{
    return cur_thread()->m_process;
}

Process& Scheduler::get_process(int pid)
{
    return (*m_process_storage)[pid];
}

}