#include "TaskManager.hpp"
#include "../drivers/DriverEntity.hpp"
#include "../drivers/DriverManager.hpp"
#include "../drivers/PIT.hpp"
#include "../fs/vfs.hpp"
#include "../hardware/descriptor_tables.hpp"
#include "../memory/kmalloc.hpp"

namespace kernel::multitasking {

extern "C" void return_from_scheduler(trapframe_t* tf);
extern "C" void return_to_the_kernel_handler(KernelContext* kc);

template <>
TaskManager* Singleton<TaskManager>::s_t = nullptr;
template <>
bool Singleton<TaskManager>::s_initialized = false;

using namespace memory;

TaskManager::TaskManager()
{
    // setup the initial kernel process
    m_kernel_process = m_process_storage.allocate_process();
    m_kernel_process->page_dir_phys = VMM::the().kernel_page_directory();
}

bool TaskManager::run()
{
    auto* pit = drivers::DriverManager::the().get_driver(drivers::DriverEntity::PIT);
    if (pit) {
        // setup the idle kernel thread, then switch to it
        add_kernel_thread([]() {
            while (1) {
            };
        });
        m_cur_thread = m_threads.begin();
        reinterpret_cast<drivers::PIT*>(pit)->register_callback({ drivers::default_frequency / 50, [](trapframe_t* tf) { TaskManager::the().schedule(tf); } });
        return_from_scheduler((*m_cur_thread)->trapframe);
    }
    return false;
}

void TaskManager::schedule(trapframe_t* tf)
{
    (*m_cur_thread)->trapframe = tf; // save context poitner

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

    // switch to the new thread's tss entry
    write_tss(GDT_KERNEL_DATA_OFFSET, (uint32_t)(*next_thread)->kernel_stack + KERNEL_STACK_SIZE);

    // swtich to the new threads's address space
    VMM::the().set_page_directory((*next_thread)->process->page_dir_phys);

    m_cur_thread = next_thread;

    return_from_scheduler((*next_thread)->trapframe);
}

void TaskManager::sys_exit_handler(int error_code)
{
    destroy_prcoess((*m_cur_thread)->process->id);
    schedule(nullptr);
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
    new_thread->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    new_thread->user_stack = kmalloc(USER_STACK_SIZE);

    // setup initial trapframe
    trapframe_t* trapframe = (trapframe_t*)new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t);
    trapframe->ds = GDT_KERNEL_DATA_OFFSET;
    trapframe->es = GDT_KERNEL_DATA_OFFSET;
    trapframe->fs = GDT_KERNEL_DATA_OFFSET;
    trapframe->gs = GDT_KERNEL_DATA_OFFSET;

    trapframe->eip = (uint32_t)func;
    trapframe->cs = GDT_KERNEL_CODE_OFFSET;
    trapframe->eflags = 0x202;
    trapframe->useresp = (uint32_t)new_thread->user_stack + USER_STACK_SIZE;
    trapframe->ss = GDT_KERNEL_DATA_OFFSET;

    new_thread->trapframe = trapframe;

    m_threads.push_front(new_thread);
}

void TaskManager::create_process(const String& filepath)
{
    auto& vmm = VMM::the();

    const uint32_t page_dir_phys = vmm.clone_page_directory();
    auto exec_data = m_elf.load_exec(filepath, page_dir_phys);

    if (!exec_data) {
        return;
    }

    Process* new_proc = m_process_storage.allocate_process();
    new_proc->page_dir_phys = page_dir_phys;

    Thread* new_thread = new Thread;
    new_thread->process = new_proc;
    new_thread->state = ThreadState::Running;
    new_thread->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    new_thread->user_stack = kmalloc(USER_STACK_SIZE);

    // setup initial trapframe
    trapframe_t* trapframe = (trapframe_t*)new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t);
    trapframe->ds = GDT_KERNEL_DATA_OFFSET;
    trapframe->es = GDT_KERNEL_DATA_OFFSET;
    trapframe->fs = GDT_KERNEL_DATA_OFFSET;
    trapframe->gs = GDT_KERNEL_DATA_OFFSET;
    trapframe->cs = GDT_KERNEL_CODE_OFFSET;
    trapframe->ss = GDT_KERNEL_DATA_OFFSET;
    trapframe->eip = exec_data.result().entry_point;
    trapframe->eflags = 0x202;
    trapframe->useresp = (uint32_t)new_thread->user_stack + USER_STACK_SIZE;

    new_thread->trapframe = trapframe;

    m_threads.push_front(new_thread);
    new_proc->m_threads.push_front(new_thread);
}

}