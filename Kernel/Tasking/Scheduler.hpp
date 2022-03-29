#pragma once

#include "Blocker.hpp"

#include <Drivers/PIT.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Libkernel/Assert.hpp>

#include <Macaronlib/List.hpp>

namespace Kernel::Tasking {

using namespace Drivers;

class ProcessStorage;
class Process;
class Thread;

class Scheduler : public TickReciever {
    friend class Process;

public:
    static Scheduler& the()
    {
        static Scheduler the {};
        return the;
    }

    bool run();

    inline bool running() const { return m_running; }
    Thread* cur_thread();
    Process* cur_process();

    // syscalls handlers
    void sys_exit_handler(int error_code);
    int sys_fork_handler();
    int sys_execve_handler(const char* filename, const char* const* argv, const char* const* envp);

    void create_process(const String& filepath);
    void reschedule();

    void on_tick(Trapframe* tf) override
    {
        reschedule();
    }

    Process& get_process(int pid);

    void block_current_thread_on_read(FileSystem::FileDescription& fd);
    void block_current_thread_on_write(FileSystem::FileDescription& fd);
    void block_current_thread();

    void unblock_threads();
    void unblock_therads_on_read();
    void unblock_therads_on_write();
    void unblock_blocker(Blocker&);

private:
    Scheduler();

private:
    List<Thread*>::Iterator find_next_thread();
    void prepare_switching_to_the_next_thread(List<Thread*>::Iterator next_thread);

    // TODO: support kernel processes / threads
    ProcessStorage* m_process_storage {};
    List<Thread*> m_threads {};
    List<Thread*>::Iterator m_cur_thread { m_threads.end() };

    bool m_running {};

    List<ReadBlocker> m_read_blockers {};
    List<WriteBlocker> m_write_blockers {};
};

}
