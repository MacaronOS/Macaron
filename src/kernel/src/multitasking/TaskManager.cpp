#include "TaskManager.hpp"
#include "../drivers/DriverEntity.hpp"
#include "../drivers/DriverManager.hpp"
#include "../drivers/PIT.hpp"
#include "../hardware/descriptor_tables.hpp"
#include "../fs/vfs.hpp"
#include "../memory/kmalloc.hpp"

namespace kernel::multitasking {

extern "C" void return_from_scheduler(trapframe_t* tf);
extern "C" void return_to_the_kernel_handler(kernel_context_t* kc);

template <>
TaskManager* Singleton<TaskManager>::s_t = nullptr;
template <>
bool Singleton<TaskManager>::s_initialized = false;

bool TaskManager::run()
{
    auto* pit = drivers::DriverManager::the().get_driver(drivers::DriverEntity::PIT);
    if (pit) {
        reinterpret_cast<drivers::PIT*>(pit)->register_callback({ drivers::default_frequency / 50, [](trapframe_t* tf) { TaskManager::the().schedule(tf); } });
        return true;
    }
    return false;
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

void TaskManager::create_process(const String& filepath) {
    auto& vfs = fs::VFS::the();
    auto& vmm = VMM::the();

    auto fd_or_error = vfs.open(filepath, 1);
    if (!fd_or_error) {
        return;
    }

    auto bianry_size_res = vfs.file_size(fd_or_error.result());
    if (!bianry_size_res) {
        return;
    }

    size_t binary_size = bianry_size_res.result();

    Process* new_proc = new Process;
    new_proc->id = m_process_count++;
    new_proc->page_dir_phys = vmm.clone_page_directory();

    // allocate space for the binary
    const size_t bin_pages_count = binary_size / 4096 + (binary_size % 4096 != 0);
    for (size_t page = 0; page < bin_pages_count; page++) {
        vmm.create_frame(new_proc->page_dir_phys, page * 4096);
    }
    // alocate space for the bss
    vmm.create_frame(new_proc->page_dir_phys, bin_pages_count);
    // load the binary
    vmm.set_page_directory(new_proc->page_dir_phys);
    vfs.read(fd_or_error.result(), (void*)0, binary_size);

    Thread* new_thread = new Thread;
    new_thread->process = new_proc;
    new_thread->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    new_thread->user_stack = kmalloc(USER_STACK_SIZE);

    // setup initial trapframe
    trapframe_t* trapframe = (trapframe_t*)new_thread->kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t);
    trapframe->ds = GDT_KERNEL_DATA_OFFSET;
    trapframe->es = GDT_KERNEL_DATA_OFFSET;
    trapframe->fs = GDT_KERNEL_DATA_OFFSET;
    trapframe->gs = GDT_KERNEL_DATA_OFFSET;

    trapframe->eip = (uint32_t)0;
    trapframe->cs = GDT_KERNEL_CODE_OFFSET;
    trapframe->eflags = 0x202;
    trapframe->useresp = (uint32_t)new_thread->user_stack + USER_STACK_SIZE;
    trapframe->ss = GDT_KERNEL_DATA_OFFSET;

    new_thread->trapframe = trapframe;

    m_threads.push_front(new_thread);
}
}