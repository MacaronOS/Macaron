#include "Process.hpp"
#include <Devices/DeviceManager.hpp>
#include <Tasking/Elf/DemandElfLoader.hpp>
#include <Tasking/MemoryDescription/AnonVMArea.hpp>
#include <Tasking/Scheduler/Scheduler.hpp>

namespace Kernel::Tasking {

using namespace Devices;

Process::ProcessPool Process::s_process_pool;

KErrorOr<Process*> Process::fork()
{
    auto allocation_result = s_process_pool.allocate();
    auto process = allocation_result.object;
    auto id = allocation_result.id;

    if (!process) {
        return KError(EAGAIN);
    }

    process->m_id = id;
    process->m_file_desciptions = m_file_desciptions;
    process->m_memory_description.fork_from(m_memory_description);

    // User stack is already presented in the forked process memory description.
    auto thread_create_result = process->create_thread(AllocateUserStack::No);
    if (!thread_create_result) {
        return thread_create_result.error();
    }

    auto thread = thread_create_result.result();
    process->set_current_thread(thread);
    thread->fork_from(current_thread());

    Scheduler::the().add_thread_for_schedulling(thread);
    return process;
}

KError Process::exec(const String& binary)
{
    auto thread_it = m_threads.begin();
    if (!thread_it) {
        ASSERT_PANIC("[Process] Can not perform exec with no threads left.");
    }

    while (m_threads.size() > 1) {
        m_threads.pop_back();
    }

    auto thread = *thread_it;
    set_current_thread(thread);
    free_memory_except_current_thread_stack();

    auto entry_point = DemandElfLoader(*this, binary).load();
    if (!entry_point) {
        return entry_point.error();
    }

    thread->setup_trapframe();
    thread->trapframe()->eip = entry_point.result();

    return KError(0);
}

void Process::terminate()
{
    auto thread_it = m_threads.rbegin();
    while (thread_it != m_threads.begin()) {
        Scheduler::the().remove_thread_from_schedulling(*thread_it);
        thread_it = m_threads.remove(thread_it);
    }
    m_memory_description.free_memory();
    s_process_pool.deallocate(m_id);
}

Thread& Process::current_thread()
{
    if (!m_current_thread) {
        ASSERT_PANIC("[Process] Current thread is not set.");
    }
    return *m_current_thread;
}

KErrorOr<Thread*> Process::create_thread(AllocateUserStack allocate)
{
    uintptr_t user_stack = 0;
    if (allocate == AllocateUserStack::Yes) {
        auto stack_area = m_memory_description.allocate_memory_area<AnonVMArea>(user_stack_size, VM_READ | VM_WRITE);
        if (!stack_area) {
            return stack_area.error();
        }

        VMM::the().set_translation_table(m_memory_description.memory_descriptor());
        VMM::the().allocate_memory_from(
            stack_area.result()->vm_start(),
            stack_area.result()->vm_size(),
            Flags::User | Flags::Write | Flags::Present);

        user_stack = stack_area.result()->vm_start();
    }

    m_threads.push_back(new Thread(*this, user_stack));
    auto thread_it = m_threads.rbegin();
    return *thread_it;
}

void Process::free_memory_except_current_thread_stack()
{
    auto& thread = current_thread();
    m_memory_description.free_memory_areas([&](VMArea& area) {
        if (area.vm_start() == thread.user_stack()) {
            return MemoryDescription::FreeMemoryArea::No;
        }
        return MemoryDescription::FreeMemoryArea::Yes;
    });
}

void Process::create_initial_process()
{
    auto allocation_result = s_process_pool.allocate();
    auto process = allocation_result.object;
    auto id = allocation_result.id;

    if (!process) {
        ASSERT_PANIC("[Process] Could not allocate an initial process");
    }

    process->m_id = id;

    auto thread_create_result = process->create_thread(AllocateUserStack::Yes);
    if (!thread_create_result) {
        ASSERT_PANIC("[Process] Could not create a thread for an initial process");
    }

    auto& thread_it = thread_create_result.result();
    process->set_current_thread(thread_it);

    auto entry_point = DemandElfLoader(*process, "/System/System").load();
    if (!entry_point) {
        ASSERT_PANIC("[Process] Could not load initial process binary");
    }

    auto& thread = *thread_it;
    thread.trapframe()->eip = entry_point.result();

    // Set standart file descriptors of the initial process to point to the debug console.
    auto console = DeviceManager::the().get_device(5, 1, DeviceType::Char);
    auto setup_file_description = [console, process]() {
        auto file_descriptor = process->file_descriptions().allocate();
        if (!file_descriptor) {
            ASSERT_PANIC("[Process] Could not allocate a standard file description for an initial process");
        }
        auto& file_description = process->file_descriptions().lookup_no_check(file_descriptor.result());
        file_description.file = console;
    };
    setup_file_description(); // stdin
    setup_file_description(); // stdout
    setup_file_description(); // stderr

    Scheduler::the().add_thread_for_schedulling(thread_it);
}

KErrorOr<Process*> Process::find_process_by_id(int id)
{
    auto process = s_process_pool.get(id);
    if (!process) {
        return KError(ESRCH);
    }
    return process;
}

}