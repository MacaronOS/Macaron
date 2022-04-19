#pragma once
#include "MemoryDescription/MemoryDescription.hpp"
#include "Scheduler.hpp"
#include "Thread.hpp"

#include <Filesystem/Base/File.hpp>
#include <Memory/Region.hpp>
#include <Memory/vmm.hpp>

#include <Macaronlib/Memory.hpp>
#include <Macaronlib/StaticStack.hpp>

#define PAGE_SIZE 4096
#define FRAME_SIZE 4096

typedef uint8_t pid_t;

namespace Kernel::Tasking {

using namespace FileSystem;
using namespace Memory;

// TODO: by increasing this value, GCC compilation slows down
constexpr pid_t MAX_PROCESSES_ALLOWED = 20;

class ProcessStorage;
class Scheduler;
class Thread;

class Process {
    friend class ProcessStorage;
    friend class Thread;
    friend class Scheduler;

public:
    inline uint32_t id() const { return m_id; }

public:
    void Terminate();
    Process* Fork();
    void LoadAndPrepare(const String& binary);

public:
    void psized_map(uint32_t page, uint32_t frame, uint32_t pages, uint32_t flags);
    void psized_allocate_space_from(uint32_t start_page, uint32_t pages, uint32_t flags);

    KErrorOr<uint32_t> psized_allocate_space(uint32_t pages, uint32_t flags, Region::Mapping mapping = Region::Mapping::Shared);
    KErrorOr<uint32_t> psized_find_free_space(uint32_t pages) const;

    inline void map(uint32_t virt_addr, uint32_t phys_addr, uint32_t sz, uint32_t flags)
    {
        psized_map(virt_addr / PAGE_SIZE, phys_addr / FRAME_SIZE, (sz + PAGE_SIZE - 1) / PAGE_SIZE, flags);
    }

    inline void allocate_space_from(uint32_t start_addr, uint32_t sz, uint32_t flags)
    {
        psized_allocate_space_from(start_addr / PAGE_SIZE, (sz + PAGE_SIZE - 1) / PAGE_SIZE, flags);
    }

    inline KErrorOr<uint32_t> allocate_space(uint32_t sz, uint32_t flags, Region::Mapping mapping = Region::Mapping::Shared)
    {
        auto page_or_error = psized_allocate_space((sz + PAGE_SIZE - 1) / PAGE_SIZE, flags, mapping);
        if (page_or_error) {
            return page_or_error.result() * PAGE_SIZE;
        }
        return page_or_error;
    }

    KErrorOr<uint32_t> find_free_space(uint32_t sz) const;

    FileDescription* file_description(fd_t fd);
    KError free_file_descriptor(fd_t fd);
    KErrorOr<fd_t> allocate_file_descriptor();

    MemoryDescription& memory_description() { return m_memory_description; }

private:
    Process() = default;
    explicit Process(uint32_t id);

private:
    void free_threads_except_one();

    void map_by_region(const Region& region);
    void allocate_space_from_by_region(const Region& region);
    void copy_by_region(const Region& region, uint32_t page_dir_from);

    inline void add_region(const Region& region) { m_regions.push_back(region); }
    inline void add_thread(Thread* thread) { m_threads.push_back(thread); }

    ProcessStorage* PS() const;
    List<Thread*>& TS() const;

    bool is_file_descriptor_in_use(fd_t fd);

public:
    Scheduler* m_task_manager {};

    uint32_t m_id {};
    Thread* cur_thread {};

    List<Thread*> m_threads {};
    List<Region> m_regions {};

    uint32_t m_signal_handler_ip {};

    Array<FileDescription, 32> m_file_descriptions {};
    StaticStack<fd_t, 32> m_free_file_descriptors {};

    MemoryDescription m_memory_description {};
};

// TODO: implement object pool

class ProcessStorage {
public:
    ProcessStorage();
    // allocates a process, with ID being set
    Process* allocate_process();
    void free_process(pid_t id);

    Process& operator[](pid_t pid);

    [[nodiscard]] size_t size() const { return MAX_PROCESSES_ALLOWED - m_free_ids.size(); }

private:
    uint8_t m_process_pool[MAX_PROCESSES_ALLOWED * sizeof(Process)] {};
    StaticStack<pid_t, MAX_PROCESSES_ALLOWED> m_free_ids {};
};

}