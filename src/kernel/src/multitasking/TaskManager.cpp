#include "TaskManager.hpp"

#include <Logger.hpp>

#include <drivers/DriverEntity.hpp>
#include <drivers/DriverManager.hpp>
#include <drivers/PIT.hpp>
#include <fs/vfs/vfs.hpp>
#include <hardware/descriptor_tables.hpp>
#include <memory/kmalloc.hpp>

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
    // setup the initial kernel process
    m_kernel_process = m_process_storage.allocate_process();
    m_kernel_process->page_dir_phys = VMM::the().kernel_page_directory();
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
        if (thread_ptr->state == ThreadState::Terminated) {
            delete thread_ptr;
            next_thread = m_threads.remove(next_thread);
            continue;
        }
        break;
    }

    auto next_thread_ptr = *next_thread;

    // switch to the new thread's tss entry
    set_kernel_stack((uint32_t)(next_thread_ptr->kernel_stack) + KERNEL_STACK_SIZE);

    // swtich to the new threads's address space
    VMM::the().set_page_directory(next_thread_ptr->process->page_dir_phys);

    m_cur_thread = next_thread;

    return_from_scheduler(next_thread_ptr->trapframe);
}

void TaskManager::sys_exit_handler(int error_code)
{
    Log() << "handling exit, PID: " << (*m_cur_thread)->process->id << "\n";
    destroy_prcoess((*m_cur_thread)->process->id);
    schedule(nullptr);
}

int TaskManager::sys_fork_handler()
{
    Log() << "handling fork\n";
    Thread* old_thread = (*m_cur_thread);

    Process* new_proc = m_process_storage.allocate_process();
    new_proc->page_dir_phys = VMM::the().clone_page_directory(old_thread->process->page_dir_phys);

    Thread* new_thread = new Thread;
    new_thread->process = new_proc;
    new_thread->state = ThreadState::Running;
    new_thread->user_stack = old_thread->user_stack;
    new_thread->kernel_stack = kmalloc_4(KERNEL_STACK_SIZE);

    // copy the trapframe
    trapframe_t* trapframe = (trapframe_t*)((uint32_t)new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t));
    *trapframe = *old_thread->trapframe;

    trapframe->eax = 0; // child process gets 0 return value
    new_thread->trapframe = trapframe;

    m_threads.push_front(new_thread);
    new_proc->m_threads.push_front(new_thread);

    return new_proc->id;
}

int TaskManager::sys_execve_handler(const char* filename, const char* const* argv, const char* const* envp)
{
    setup_process((*m_cur_thread)->process->id, filename);
    return 1;
}

void TaskManager::destroy_prcoess(const pid_t pid)
{
    Process& process = m_process_storage[pid];

    // mark all process's threds as terminated, so they will be freed
    // when scheduler finds them
    for (Thread* thread : process.m_threads) {
        thread->state = ThreadState::Terminated;
    }

    m_process_storage.free_process(pid);
}

void TaskManager::add_kernel_thread(void (*func)())
{
    Thread* new_thread = new Thread;
    new_thread->process = kernel_process();
    new_thread->kernel_stack = kmalloc_4(KERNEL_STACK_SIZE);
    new_thread->user_stack = kmalloc_4(USER_STACK_SIZE);

    // setup initial trapframe
    trapframe_t* trapframe = (trapframe_t*)((uint32_t)new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t));
    trapframe->ds = GDT_KERNEL_DATA_OFFSET;
    trapframe->es = GDT_KERNEL_DATA_OFFSET;
    trapframe->fs = GDT_KERNEL_DATA_OFFSET;
    trapframe->gs = GDT_KERNEL_DATA_OFFSET;
    trapframe->ss = GDT_KERNEL_DATA_OFFSET;
    trapframe->cs = GDT_KERNEL_CODE_OFFSET;

    trapframe->eip = (uint32_t)func;
    trapframe->eflags = 0x202;
    trapframe->useresp = (uint32_t)new_thread->user_stack + USER_STACK_SIZE;

    new_thread->trapframe = trapframe;

    m_threads.push_front(new_thread);
}

void TaskManager::create_process(const String& filepath)
{
    setup_process(m_process_storage.allocate_process()->id, filepath);
}

void TaskManager::setup_process(const pid_t pid, const String& filepath)
{

    auto& vmm = VMM::the();
    auto& proc = m_process_storage[pid];

    if (!proc.page_dir_phys) {
        proc.page_dir_phys = vmm.clone_page_directory();
    } else {
        vmm.clear_user_directory_pages(proc.page_dir_phys);
    }

    auto exec_data = m_elf.load_exec(filepath, proc.page_dir_phys);

    if (!exec_data) {
        return;
    }

    while (proc.m_threads.size() > 1) {
        m_threads.remove(m_threads.find(*proc.m_threads.rbegin()));
        proc.m_threads.remove(proc.m_threads.rbegin());
    }

    if (proc.m_threads.size() < 1) {
        auto new_thread = new Thread;
        new_thread->process = &proc;
        new_thread->kernel_stack = kmalloc_4(KERNEL_STACK_SIZE);
        proc.m_threads.push_back(new_thread);
        m_threads.push_front(new_thread);
    }

    auto thread = *proc.m_threads.begin();
    thread->state = ThreadState::Running;

    // place user stack just before the kernel
    vmm.create_frame(proc.page_dir_phys, HIGHER_HALF_OFFSET - USER_STACK_SIZE);
    thread->user_stack = (void*)(HIGHER_HALF_OFFSET - USER_STACK_SIZE);

    // setup initial trapframe
    trapframe_t* trapframe = (trapframe_t*)((uint32_t)thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t));
    trapframe->ds = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
    trapframe->es = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
    trapframe->fs = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
    trapframe->gs = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
    trapframe->cs = GDT_USER_CODE_OFFSET | REQUEST_RING_3;
    trapframe->ss = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
    trapframe->eip = exec_data.result().entry_point;
    trapframe->eflags = 0x202;
    trapframe->useresp = (uint32_t)thread->user_stack + USER_STACK_SIZE;
    trapframe->ebp = trapframe->useresp;

    thread->trapframe = trapframe;
}

}