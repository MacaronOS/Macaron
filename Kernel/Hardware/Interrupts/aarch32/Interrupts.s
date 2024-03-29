.section .text

.global prefetch_abort_exception_handler
prefetch_abort_exception_handler:
    // lr_ABT points to the instruction at the address following the one 
    // that caused the abort, so the address to be restored is at lr_ABT-4.
    sub lr, #4
    stmfd sp!, {r0-r12, lr}
    mrs r2, spsr
    mrs r1, sp_usr
    mrs r0, lr_usr
    stmfd sp!, {r0-r2}

    ldr r1, =data_abort_handler
    mov r0, sp
    blx r1

    b exception_out

.global data_abort_exception_handler
data_abort_exception_handler:
    // The instruction that caused the abort is at lr_ABT-8 because
    // lr_ABT points two instructions beyond the instruction that caused the abort.
    sub lr, #8
    stmfd sp!, {r0-r12, lr}
    mrs r2, spsr
    mrs r1, sp_usr
    mrs r0, lr_usr
    stmfd sp!, {r0-r2}

    ldr r1, =data_abort_handler
    mov r0, sp
    blx r1

    b exception_out

.global irq_exception_handler
irq_exception_handler:
    // Saving IRQ-banked (and R0) registers on the IRQ stack.
    sub lr, #4
    str lr, [sp, #0] // LR
    mrs lr, spsr
    str lr, [sp, #-4] // SPSR
    mov lr, r0
    str lr, [sp, #-8] // R0

    // Saving IRQ-banked stack pointer in R0 to access stored registers above.
    mov r0, sp
    // Moving to the Supervisor mode.
    cps #0x13

    // Building the trapframe.
    // See Hardware/aarch32/Trapframe.hpp.
    ldr lr, [r0, #0]
    stmfd sp!, {r1-r12, lr}
    ldr r12, [r0, #-8]
    ldr r11, [r0, #-4]
    mrs r10, sp_usr
    mrs r9, lr_usr
    stmfd sp!, {r9-r12}

    ldr r1, =irq_handler
    mov r0, sp
    blx r1

exception_out:
    // Restoring registers from the trapframe.
    ldmfd sp!, {r0-r2}
    msr lr_usr, r0
    msr sp_usr, r1
    msr spsr, r2
    ldmfd sp!, {r0-r12, pc}^
