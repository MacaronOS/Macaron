#include "Scheduler.hpp"
#include <Devices/DeviceManager.hpp>
#include <Drivers/Base/DriverEntity.hpp>
#include <Drivers/DriverManager.hpp>
#include <Drivers/PIT.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Hardware/x86/DescriptorTables/GDT.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/AnonVMArea.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>
#include <Tasking/Processes/Threads/Signal.hpp>

namespace Kernel::Tasking {

extern "C" void switch_to_user_mode();

// Next 2 functions are used when a scheduler decides to switch to the new thread.
// The new thread could be preempted either in kernel space or user space.
//
// If it was preempted in kernel space, switch_to_kernel should be
// called to resume the thread. Othrevise, switch_to_user.
extern "C" void switch_to_kernel(KernelContext** kc);
extern "C" void switch_to_user(Trapframe* tf);

// Next 2 functions are used when a thread blocks inside the kernel.
// There are also 2 versions in analogy with the previous 2 functions.
extern "C" void block_and_switch_to_kernel(KernelContext** cur, KernelContext** next);
extern "C" void block_and_switch_to_user(KernelContext** cur, Trapframe* next);

using namespace Memory;
using namespace Logger;
using namespace Devices;

Thread& Scheduler::current_thread()
{
    if (!m_current_thread) {
        ASSERT_PANIC("[Scheduler] Current thread is not set.");
    }
    return **m_current_thread;
}

Process& Scheduler::current_process()
{
    return current_thread().process();
}

void Scheduler::initialize()
{
    Signals::setup_caller();
    Process::create_initial_process();
    m_current_thread = m_schedulling_threads.begin();
}

void Scheduler::run()
{
    m_running = true;
    PIT::the().register_tick_reciever(this);
    switch_to_user_mode();
    reschedule();
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

Scheduler::Iterator Scheduler::find_next_thread()
{
    auto next_thread = m_current_thread;
    next_thread++;
    if (next_thread == m_schedulling_threads.end()) {
        next_thread = m_schedulling_threads.begin();
    }
    return next_thread;
}

void Scheduler::prepare_switching_to_the_next_thread(Iterator next_thread_for_scheduling)
{
    auto next_thread_it = *next_thread_for_scheduling;
    auto& next_thread = *next_thread_it;

    // switch to the new thread's tss entry
    DescriptorTables::GDT::SetKernelStack(next_thread.kernel_stack_top());

    // swtich to the new process address space
    VMM::the().set_translation_table(next_thread.process().memory_description().memory_descriptor());

    next_thread.process().set_current_thread(next_thread_it);

    m_current_thread = next_thread_for_scheduling;

    next_thread.signals().dispatch_pending([&](int signo) {
        if (next_thread.signals().handler(signo)) {
            next_thread.jump_to_signal_caller(signo);
        } else {
            auto action = default_action(signo);

            if (action == DefaultAction::Terminate) {
                next_thread.process().terminate();
                reschedule();
                return CallbackIterate::Stop;
            }
        }
        return CallbackIterate::Continue;
    });
}

void Scheduler::block_current_thread()
{
    auto& thread = current_thread();
    m_current_thread = m_schedulling_threads.remove(m_current_thread);
    auto next_thread_it = find_next_thread();
    auto& next_thread = **next_thread_it;

    prepare_switching_to_the_next_thread(next_thread_it);

    if (next_thread.blocked_in_kernel()) {
        block_and_switch_to_kernel(thread.kernel_context(), next_thread.kernel_context());
    } else {
        block_and_switch_to_user(thread.kernel_context(), next_thread.trapframe());
    }
}

void Scheduler::unblock_blocker(Blocker& blocker)
{
    auto thread = blocker.thread();
    Logger::Log() << "unblocking pid " << thread->process().id() << "\n";
    m_schedulling_threads.push_back(thread);
}

void Scheduler::add_thread_for_schedulling(Thread* thread)
{
    m_schedulling_threads.push_back(thread);
}

void Scheduler::remove_thread_from_schedulling(Thread* thread)
{
    m_schedulling_threads.remove(m_schedulling_threads.find(thread));
    if ((*m_current_thread) == thread) {
        m_current_thread = m_schedulling_threads.begin();
    }
}

}