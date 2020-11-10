#include "syscalls.hpp"
#include "assert.hpp"
#include "monitor.hpp"
#include "hardware/trapframe.hpp"

static uint32_t syscalls[1] = {
    (uint32_t)term_printd,
};

uint32_t num_syscalls = 1;

void syscalls_handler(trapframe_t* regs)
{
    if (regs->eax >= num_syscalls) {
        return;
    }
    int ret;
    asm volatile(
        " \ 
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
        : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(syscalls[regs->eax]));

    regs->eax = ret;
}

void syscalls_init()
{
    term_print("init...");
}
