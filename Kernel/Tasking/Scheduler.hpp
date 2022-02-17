#pragma once

#include "Process.hpp"
#include "Thread.hpp"
#include "Blocker.hpp"

#include <Drivers/PIT.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Libkernel/Assert.hpp>
#include <Memory/vmm.hpp>

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

private:
    Scheduler();

    // TODO: support kernel processes / threads
    ProcessStorage* m_process_storage {};
    List<Thread*> m_threads {};
    List<Thread*>::Iterator m_cur_thread { m_threads.end() };
    bool m_running {};
};

}
