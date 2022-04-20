#include "Process.hpp"
#include "Elf/DemandElfLoader.hpp"
#include "Scheduler.hpp"
#include <Tasking/MemoryDescription/AnonVMArea.hpp>

#include <Libkernel/Logger.hpp>
#include <Memory/Region.hpp>

namespace Kernel::Tasking {

extern "C" void signal_caller();
extern "C" void signal_caller_end();

ProcessStorage::ProcessStorage()
{
    for (int i = MAX_PROCESSES_ALLOWED - 1; i >= 0; i--) {
        m_free_ids.push(i);
    }
}

Process* ProcessStorage::allocate_process()
{
    if (!m_free_ids.size()) {
        return nullptr;
    }
    auto free_id = m_free_ids.top_and_pop();
    new (&m_process_pool[free_id * sizeof(Process)]) Process(free_id);
    return (Process*)&m_process_pool[free_id * sizeof(Process)];
}

void ProcessStorage::free_process(pid_t id)
{
    ((Process*)(&m_process_pool[id * sizeof(Process)]))->~Process();
    m_free_ids.push(id);
}

Process& ProcessStorage::operator[](pid_t pid)
{

    return *((Process*)(&m_process_pool[pid * sizeof(Process)]));
}

Process::Process(uint32_t id)
    : m_id(id)
{
    m_task_manager = &Scheduler::the();

    // Setup signal caller
    uint32_t signal_caller_len = (uint32_t)signal_caller_end - (uint32_t)signal_caller;
    auto space = allocate_space(signal_caller_len, Flags::User | Flags::Write | Flags::Present);
}

void Process::Terminate()
{
    for (auto thread : m_threads) {
        thread->Terminate();
    }
    PS()->free_process(id());
}

Process* Process::Fork()
{
    auto new_proc = PS()->allocate_process();
    if (!new_proc) {
        return nullptr;
    }

    new_proc->m_file_descriptions = m_file_descriptions;
    new_proc->m_free_file_descriptors = m_free_file_descriptors;
    new_proc->m_memory_description.fork_from(m_memory_description);

    auto new_thread = Thread::TieNewTo(new_proc);
    new_thread->set_user_stack(cur_thread->user_stack());
    *new_thread->trapframe() = *cur_thread->trapframe();
    new_thread->trapframe()->eax = 0;

    return new_proc;
}

void Process::LoadAndPrepare(const String& binary)
{
    free_threads_except_one();
    m_memory_description.free_memory();

    auto stack_area = m_memory_description.allocate_memory_area<AnonVMArea>(USER_STACK_SIZE, VM_READ | VM_WRITE);
    if (!stack_area) {
        return;
    }

    VMM::the().allocate_space_from(
        m_memory_description.memory_descriptor(),
        stack_area.result()->vm_start(),
        stack_area.result()->vm_size(),
        Flags::User | Flags::Write | Flags::Present);

    auto entry_point = DemandElfLoader(*this, binary).load();
    if (!entry_point) {
        return;
    }

    auto initial_thread = *m_threads.begin();
    initial_thread->set_user_stack(stack_area.result()->vm_start());
    initial_thread->trapframe()->eip = entry_point.result();
}

ProcessStorage* Process::PS() const
{
    return m_task_manager->m_process_storage;
}

List<Thread*>& Process::TS() const
{
    return m_task_manager->m_threads;
}

KErrorOr<uint32_t> Process::psized_allocate_space(uint32_t pages, uint32_t flags, Region::Mapping mapping)
{
    auto page_or_error = VMM::the().psized_find_free_space(m_memory_description.memory_descriptor(), pages);
    if (page_or_error) {
        allocate_space_from_by_region({
            .type = Region::Type::Allocated,
            .mapping = mapping,
            .page = page_or_error.result(),
            .pages = pages,
            .flags = flags,
        });
    }
    return page_or_error;
}

void Process::psized_allocate_space_from(uint32_t start_page, uint32_t pages, uint32_t flags)
{
    allocate_space_from_by_region({
        .type = Region::Type::Allocated,
        .page = start_page,
        .pages = pages,
        .flags = flags,
    });
}

void Process::psized_map(uint32_t page, uint32_t frame, uint32_t pages, uint32_t flags)
{
    map_by_region({
        .type = Region::Type::Mapping,
        .page = page,
        .frame = frame,
        .pages = pages,
        .flags = flags,
    });
}

KErrorOr<uint32_t> Process::psized_find_free_space(uint32_t pages) const
{
    return VMM::the().psized_find_free_space(m_memory_description.memory_descriptor(), pages);
}

KErrorOr<uint32_t> Process::find_free_space(uint32_t sz) const
{
    return VMM::the().find_free_space(m_memory_description.memory_descriptor(), sz);
}

void Process::allocate_space_from_by_region(const Region& region)
{
    VMM::the().psized_allocate_space_from(m_memory_description.memory_descriptor(), region.page, region.pages, region.flags);
    add_region(region);
}

void Process::copy_by_region(const Region& region, uint32_t page_dir_from)
{
    VMM::the().psized_copy(m_memory_description.memory_descriptor(), page_dir_from, region.page, region.pages);
    add_region(region);
}

void Process::map_by_region(const Region& region)
{
    VMM::the().psized_map(m_memory_description.memory_descriptor(), region.page, region.frame, region.pages, region.flags);
    add_region(region);
}

void Process::free_threads_except_one()
{
    if (m_threads.size() > 1) {
        auto it = m_threads.rbegin();
        while (it != m_threads.rend()) {
            auto thread = *it;
            TS().remove(TS().find(thread));
        }
    } else if (m_threads.size() < 1) {
        Thread::TieNewTo(this);
        return;
    }

    auto thread = *m_threads.begin();
    thread->reset_stack();
    thread->reset_flags();
}

FileDescription* Process::file_description(fd_t fd)
{
    if (!is_file_descriptor_in_use(fd)) {
        return nullptr;
    }

    return &m_file_descriptions[fd];
}

KError Process::free_file_descriptor(fd_t fd)
{
    if (!is_file_descriptor_in_use(fd)) {
        return KError(EBADF);
    }

    m_free_file_descriptors.push(fd);

    return KError(0);
}

KErrorOr<fd_t> Process::allocate_file_descriptor()
{
    if (m_free_file_descriptors.empty()) {
        return KError(EBADFD);
    }

    auto free_fd = m_free_file_descriptors.top_and_pop();
    m_file_descriptions[free_fd].file = nullptr;
    m_file_descriptions[free_fd].flags = 0;
    m_file_descriptions[free_fd].offset = 0;

    return free_fd;
}

bool Process::is_file_descriptor_in_use(fd_t fd)
{
    for (auto fdi : m_free_file_descriptors) {
        if (fdi == fd) {
            return false;
        }
    }

    return true;
}

}