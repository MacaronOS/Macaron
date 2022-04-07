#include "Scheduler.hpp"
#include "Process.hpp"
#include "Signal.hpp"
#include <Devices/DeviceManager.hpp>
#include <Drivers/Base/DriverEntity.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Hardware/DescriptorTables/GDT.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Tasking {

extern "C" void switch_to_user_mode();

// Next 2 functions are used when a scheduler decides to switch to the new thread.
// The new thread could be preempted either in kernel space or user space.
//
// If it was preempted in kernel space, block_and_switch_to_kernel should be
// called to resume the thread. Othrevise, block_and_switch_to_user.
extern "C" void switch_to_kernel(KernelContext** kc);
extern "C" void switch_to_user(Trapframe* tf);

// Next 2 functions are used when a thread blocks inside the kernel.
// There are also 2 versions in analogy with the previous 2 functions.
extern "C" void block_and_switch_to_kernel(KernelContext** cur, KernelContext** next);
extern "C" void block_and_switch_to_user(KernelContext** cur, Trapframe* next);

using namespace Memory;
using namespace Logger;
using namespace Devices;

Scheduler::Scheduler()
{
    m_process_storage = new ProcessStorage();
}

bool Scheduler::run()
{
    m_running = true;
    m_cur_thread = m_threads.begin();

    auto process = cur_process();

    for (int i = process->m_file_descriptions.size() - 1; i >= 0; i--) {
        process->m_free_file_descriptors.push(i);
    }

    // Set standart file descriptors of the initial process to point to the debug console.
    auto console = DeviceManager::the().get_device(5, 1, DeviceType::Char);

    auto& stdin = *process->file_description(process->allocate_file_descriptor().result());
    auto& stdout = *process->file_description(process->allocate_file_descriptor().result());
    auto& stderr = *process->file_description(process->allocate_file_descriptor().result());

    stdin.file = console;
    stdout.file = console;
    stderr.file = console;

    PIT::the().register_tick_reciever(this);
    switch_to_user_mode();
    reschedule();
    return false;
}

void Scheduler::reschedule()
{
    unblock_threads();
    auto next_thread = find_next_thread();
    prepare_switching_to_the_next_thread(next_thread);

    auto next_thread_ptr = *next_thread;
    if (next_thread_ptr->blocked_in_kernel()) {
        switch_to_kernel(next_thread_ptr->kernel_context());
    } else {
        switch_to_user(next_thread_ptr->trapframe());
    }
}

List<Thread*>::Iterator Scheduler::find_next_thread()
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

    return next_thread;
}

void Scheduler::prepare_switching_to_the_next_thread(List<Thread*>::Iterator next_thread)
{
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

void Scheduler::block_current_thread()
{
    auto cur_thread_ptr = *m_cur_thread;
    m_cur_thread = m_threads.remove(m_cur_thread);
    auto next_thread = find_next_thread();
    auto next_thread_ptr = *next_thread;

    prepare_switching_to_the_next_thread(next_thread);

    if (next_thread_ptr->blocked_in_kernel()) {
        block_and_switch_to_kernel(cur_thread_ptr->kernel_context(), next_thread_ptr->kernel_context());
    } else {
        block_and_switch_to_user(cur_thread_ptr->kernel_context(), next_thread_ptr->trapframe());
    }
}

void Scheduler::unblock_blocker(Blocker& blocker)
{
    auto thread = blocker.thread();
    Logger::Log() << "unblocking pid " << thread->m_process->id() << "\n";
    m_threads.push_back(thread);
}

}