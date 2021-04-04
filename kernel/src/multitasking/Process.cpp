#include "Process.hpp"
#include "TaskManager.hpp"

#include <Logger.hpp>
#include <memory/Region.hpp>

namespace kernel::multitasking {

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
    m_task_manager = &TaskManager::the();
    m_pdir_phys = VMM::the().create_page_directory();
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

    for (auto region : m_regions) {
        if (region.type == Region::Type::Mapping) {
            new_proc->map_by_region(region);
        } else {
            if (region.mapping == Region::Mapping::Anonimous) {
                continue;
            }
            new_proc->copy_by_region(region, m_pdir_phys);
        }
    }

    auto new_thread = Thread::TieNewTo(new_proc);

    auto stack_buffer = (uint8_t*)malloc(USER_STACK_SIZE);
    VMM::the().set_page_directory(m_pdir_phys);
    for (size_t i = 0; i < USER_STACK_SIZE; i++) {
        stack_buffer[i] = ((uint8_t*)cur_thread->user_stack_ptr())[i];
    }
    VMM::the().set_page_directory(new_proc->m_pdir_phys);
    for (size_t i = 0; i < USER_STACK_SIZE; i++) {
        ((uint8_t*)new_thread->user_stack_ptr())[i] = stack_buffer[i];
    }
    free(stack_buffer);

    *new_thread->trapframe() = *cur_thread->trapframe();
    new_thread->trapframe()->eax = 0;

    return new_proc;
}

void Process::LoadAndPrepare(const String& binary)
{
    free_threads_except_one();
    auto initial_thread = *m_threads.begin();
    free_regions_except_include(initial_thread->user_stack() / PAGE_SIZE);

    auto elf = Elf::load_exec(binary, m_pdir_phys);
    if (!elf) {
        return;
    }
    m_regions.push_back(*elf.result().regions.begin());
    initial_thread->trapframe()->eip = elf.result().entry_point;
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
    auto page_or_error = VMM::the().psized_find_free_space(m_pdir_phys, pages);
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
    return VMM::the().psized_find_free_space(m_pdir_phys, pages);
}

KErrorOr<uint32_t> Process::find_free_space(uint32_t sz) const
{
    return VMM::the().find_free_space(m_pdir_phys, sz);
}

void Process::allocate_space_from_by_region(const Region& region)
{
    VMM::the().psized_allocate_space_from(m_pdir_phys, region.page, region.pages, region.flags);
    add_region(region);
}

void Process::copy_by_region(const Region& region, uint32_t page_dir_from)
{
    VMM::the().psized_copy(m_pdir_phys, page_dir_from, region.page, region.pages);
    add_region(region);
}

void Process::map_by_region(const Region& region)
{
    VMM::the().psized_map(m_pdir_phys, region.page, region.frame, region.pages, region.flags);
    add_region(region);
}

void Process::free_regions_except_include(uint32_t page)
{
    auto it = m_regions.rbegin();
    while (it != m_regions.rend()) {
        auto region = *it;
        if (region.includes(page)) {
            it--;
            continue;
        }
        if (region.type == Region::Type::Mapping) {
            VMM::the().unmap(m_pdir_phys, region.page, region.pages);
        } else {
            VMM::the().free(m_pdir_phys, region.page, region.pages);
        }

        it = m_regions.remove(it);
    }
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

}