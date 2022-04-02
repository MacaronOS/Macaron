#include "Syscalls.hpp"

#include <FileSystem/VFS/VFS.hpp>
#include <Hardware/Trapframe.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>
#include <Tasking/Process.hpp>
#include <Tasking/Scheduler.hpp>
#include <Tasking/SharedBuffers/SharedBufferStorage.hpp>
#include <Time/TimeManager.hpp>

#include <Macaronlib/ABI/Errors.hpp>
#include <Macaronlib/ABI/Signals.hpp>
#include <Macaronlib/ABI/Syscalls.hpp>

namespace Kernel::Syscalls {

using namespace Logger;
using namespace Memory;
using namespace Tasking;
using namespace FileSystem;
using namespace Time;

static int sys_putc(char a)
{
    Log() << "handling putc " << a << "\n";
    VgaTUI::Putc(a);
    return 1;
}

static int sys_exit(int error_code)
{
    Scheduler::the().sys_exit_handler(error_code);
    return 1;
}

static int sys_fork()
{
    return Scheduler::the().sys_fork_handler();
}

static int sys_read(fd_t fd, uint8_t* buf, size_t cnt)
{
    return int(VFS::the().read(fd, buf, cnt));
}

static int sys_write(fd_t fd, uint8_t* buf, size_t cnt)
{
    return int(VFS::the().write(fd, buf, cnt));
}

static int sys_lseek(fd_t fd, size_t offset, int whence)
{
    return int(VFS::the().lseek(fd, offset, whence));
}

static int sys_open(const char* filename, int flags, unsigned short mode)
{
    Log() << "handling open " << filename << "\n";
    return int(VFS::the().open(filename, flags, mode));
}

static int sys_execve(const char* filename, const char* const* argv, const char* const* envp)
{
    Log() << "handling excve\n";
    return Scheduler::the().sys_execve_handler(filename, argv, envp);
}

static int sys_mmap(MmapParams* params)
{
    Log() << "handling mmap\n";
    auto cur_process = Scheduler::the().cur_process();

    if (params->flags & MAP_ANONYMOUS) {
        auto mem = cur_process->allocate_space(params->size, Flags::Present | Flags::Write | Flags::User);
        if (!mem) {
            // TODO: implement errno
            return -1;
        }
        return mem.result();
    }

    if (params->fd) {
        uint32_t mem = params->start;
        if (!mem) {
            auto free_space = cur_process->find_free_space(params->size);
            if (!free_space) {
                return -1;
            }
            mem = free_space.result();
        }
        auto error_happened = VFS::the().mmap(params->fd, (void*)mem, params->size);
        if (error_happened) {
            return -1;
        }
        return mem;
    }

    return -EBADF;
}

static int sys_write_string(String const* str)
{
    Logger::Log() << "PID" << Scheduler::the().cur_process()->id() << ": " << *str << "\n";
    return 0;
}

static void sys_create_shared_buffer(size_t size, CreateBufferResult* result)
{
    *result = SharedBufferStorage::the().create_buffer(size);
}

static int sys_get_shared_buffer(uint32_t id)
{
    return SharedBufferStorage::the().get_buffer(id);
}

static int sys_ioctl(fd_t fd, uint32_t request)
{
    return int(VFS::the().ioctl(fd, request));
}

static int sys_socket(int domain, int type, int protocol)
{
    return int(VFS::the().socket(domain, type, protocol));
}

static int sys_bind(fd_t fd, const char* path)
{
    return int(VFS::the().bind(fd, path));
}

static int sys_connect(fd_t fd, const char* path)
{
    return int(VFS::the().connect(fd, path));
}

static int sys_can_read(fd_t fd)
{
    return VFS::the().can_read(fd);
}

static int sys_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout)
{
    return int(VFS::the().select(nfds, readfds, writefds, execfds, timeout));
}

static int sys_getpid()
{
    return Scheduler::the().cur_process()->id();
}

static int sys_clock_gettime(int clock_id, timespec* ts)
{
    auto result = TimeManager::the().get_time(clock_id);
    if (result.error()) {
        return result.error().posix_error();
    }
    *ts = result.result();
    return 0;
}

static int sys_getdents(fd_t fd, linux_dirent* dirp, size_t size)
{
    return int(VFS::the().getdents(fd, dirp, size));
}

static int sys_sigaction(int sig, const sigaction* act, sigaction* old_act)
{
    auto cur_thread = Scheduler::the().cur_thread();
    cur_thread->set_signal_handler(sig, (void*)act->sa_handler);
    return 0;
}

static int sys_sigprocmask(int how, const sigset_t* set, sigset_t* old_set)
{
    auto cur_thread = Scheduler::the().cur_thread();
    if (old_set != nullptr) {
        *old_set = cur_thread->signal_mask();
    }
    if (how == SIG_BLOCK) {
        cur_thread->signal_mask_block(*set);
    } else if (how == SIG_UNBLOCK) {
        cur_thread->signal_mask_unblock(*set);
    } else if (how == SIG_SETMASK) {
        cur_thread->signal_mask_set(*set);
    }
    return 0;
}

static int sys_sigreturn()
{
    auto cur_thread = Scheduler::the().cur_thread();
    auto trapframe = cur_thread->trapframe();
    trapframe->useresp += sizeof(uint32_t); // cleaning signo argument
    trapframe->edi = trapframe->pop();
    trapframe->esi = trapframe->pop();
    trapframe->ebp = trapframe->pop();
    trapframe->esp = trapframe->pop();
    trapframe->ebx = trapframe->pop();
    trapframe->edx = trapframe->pop();
    trapframe->ecx = trapframe->pop();
    trapframe->eax = trapframe->pop();
    trapframe->eip = trapframe->pop();
    trapframe->eflags = trapframe->pop();
    trapframe->useresp = trapframe->pop();
    Scheduler::the().reschedule();
    return 0; // never returns
}

static int sys_kill(int pid, int sig)
{
    Scheduler::the().get_process(pid).cur_thread->signal_add_pending(sig);
    return 0;
}

static int sys_ptsname(int fd, char* buffer, size_t size)
{
    if (size == 0) {
        return 0;
    }

    auto ptsname = VFS::the().ptsname(fd);
    if (!ptsname) {
        return ptsname.error().posix_error();
    }

    if (ptsname.result().size() == 0) {
        return 0;
    }

    auto path = String("/dev/pts/") + ptsname.result();
    size_t sz = min(path.size(), size - 1);
    memcpy(buffer, path.c_str(), sz);
    buffer[sz] = '\0';

    return 0;
}

static int sys_close(int fd)
{
    return int(VFS::the().close(fd));
}

SyscallsManager::SyscallsManager()
    : InterruptHandler(0x80)
{
    register_syscall(Syscall::Putc, (uint32_t)sys_putc);
    register_syscall(Syscall::Exit, (uint32_t)sys_exit);
    register_syscall(Syscall::Fork, (uint32_t)sys_fork);
    register_syscall(Syscall::Read, (uint32_t)sys_read);
    register_syscall(Syscall::Write, (uint32_t)sys_write);
    register_syscall(Syscall::Open, (uint32_t)sys_open);
    register_syscall(Syscall::Execve, (uint32_t)sys_execve);
    register_syscall(Syscall::Mmap, (uint32_t)sys_mmap);
    register_syscall(Syscall::Ioctl, (uint32_t)sys_ioctl);
    register_syscall(Syscall::WriteString, (uint32_t)sys_write_string);
    register_syscall(Syscall::CreateSharedBuffer, (uint32_t)sys_create_shared_buffer);
    register_syscall(Syscall::GetSharedBuffer, (uint32_t)sys_get_shared_buffer);
    register_syscall(Syscall::Socket, (uint32_t)sys_socket);
    register_syscall(Syscall::Bind, (uint32_t)sys_bind);
    register_syscall(Syscall::Connect, (uint32_t)sys_connect);
    register_syscall(Syscall::CanRead, (uint32_t)sys_can_read);
    register_syscall(Syscall::Select, (uint32_t)sys_select);
    register_syscall(Syscall::GetPid, (uint32_t)sys_getpid);
    register_syscall(Syscall::Lseek, (uint32_t)sys_lseek);
    register_syscall(Syscall::ClockGettime, (uint32_t)sys_clock_gettime);
    register_syscall(Syscall::GetDents, (uint32_t)sys_getdents);
    register_syscall(Syscall::Sigaction, (uint32_t)sys_sigaction);
    register_syscall(Syscall::Sigreturn, (uint32_t)sys_sigreturn);
    register_syscall(Syscall::Kill, (uint32_t)sys_kill);
    register_syscall(Syscall::PTSName, (uint32_t)sys_ptsname);
    register_syscall(Syscall::Close, (uint32_t)sys_close);
}

void SyscallsManager::initialize()
{
    new SyscallsManager();
}

void SyscallsManager::register_syscall(Syscall ss, uint32_t syscall_ptr)
{
    m_syscalls[(uint16_t)ss] = syscall_ptr;
}

void SyscallsManager::handle_interrupt(Trapframe* regs)
{
    if (static_cast<Syscall>(regs->eax) == Syscall::SchedYield) {
        regs->eax = 0;
        Scheduler::the().reschedule();
    }
    if (regs->eax >= syscall_count || !m_syscalls[regs->eax]) {
        return;
    }
    int ret;
    asm volatile(
        "\
        push %1; \
        push %2; \
        push %3; \
        push %4; \
        push %5; \
        call *%6; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        "
        : "=a"(ret)
        : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(m_syscalls[regs->eax]));

    regs->eax = ret;
}

}