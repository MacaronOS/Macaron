#include "TaskManager.hpp"
#include "../drivers/DriverEntity.hpp"
#include "../drivers/DriverManager.hpp"
#include "../drivers/PIT.hpp"
#include "../hardware/descriptor_tables.hpp"
#include "../memory/kmalloc.hpp"

namespace kernel::multitasking {

extern "C" void return_from_scheduler(trapframe_t* tf);

TaskManager* TaskManager::s_tm = nullptr;
bool TaskManager::initialized = false;

bool TaskManager::run()
{
    auto* pit = drivers::DriverManager::the().get_driver(drivers::DriverEntity::PIT);
    if (pit) {
        reinterpret_cast<drivers::PIT*>(pit)->register_callback({ drivers::default_frequency / 50, [](trapframe_t* tf) { TaskManager::the().schedule(tf); } });
        return true;
    }
    return false;
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

void TaskManager::schedule(trapframe_t* tf)
{
    // we don't have threads at this moment
    if (m_threads.empty()) {
        return;
    }

    auto next_thread = m_cur_thread;

    // save tf position in the stack
    if (m_cur_thread != m_threads.end()) {
        (*m_cur_thread)->trapframe = tf;

        if (++next_thread == m_threads.end()) {
            next_thread = m_threads.begin();
        }
    } else {
        next_thread = m_threads.begin();
    }

    // switch to the new thread's tss entry
    write_tss(GDT_KERNEL_DATA_OFFSET, (uint32_t)(*next_thread)->kernel_stack + KERNEL_STACK_SIZE);

    // swtich to the new threads's address space
    VMM::the().set_page_directory((*next_thread)->process->page_dir_phys);

    m_cur_thread = next_thread;

    return_from_scheduler((*next_thread)->trapframe);
}

}