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
    friend class Thread;

public:
    static Scheduler& the()
    {
        static Scheduler the {};
        return the;
    }
    bool initialize();
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

    template <typename BlockerType>
    void block_current_thread_with(BlockerType&& blocker)
    {
        blocker.set_thread(cur_thread());
        if constexpr (IsSame<BlockerType, ReadBlocker>) {
            m_read_blockers.push_back(blocker);
        } else if constexpr (IsSame<BlockerType, WriteBlocker>) {
            m_write_blockers.push_back(blocker);
        }
        block_current_thread();
    }

private:
    void unblock_threads()
    {
        unblock_threads_on<ReadBlocker>();
        unblock_threads_on<WriteBlocker>();
    }

    template <typename BlockerType>
    void unblock_threads_on()
    {
        if constexpr (IsSame<BlockerType, ReadBlocker>) {
            unblock_blocker_list<List<ReadBlocker>>(m_read_blockers);
        } else if constexpr (IsSame<BlockerType, WriteBlocker>) {
            unblock_blocker_list<List<WriteBlocker>>(m_write_blockers);
        }
    }

    template <typename BlockerList>
    void unblock_blocker_list(BlockerList& blocker_list)
    {
        auto blocker = blocker_list.rbegin();
        while (blocker != blocker_list.rend()) {
            if ((*blocker).can_unblock()) {
                unblock_blocker(*blocker);
                blocker = blocker_list.remove(blocker);
                continue;
            }
            --blocker;
        }
    }

    void unblock_blocker(Blocker&);
    void block_current_thread();

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
    size_t m_signal_handler_ip {};
};

}
