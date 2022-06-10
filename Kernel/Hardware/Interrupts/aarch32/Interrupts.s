.section .text

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

    // Restoring registers from the trapframe.
    ldmfd sp!, {r0-r2}
    msr lr_usr, r0
    msr sp_usr, r1
    msr spsr, r2
    ldmfd sp!, {r0-r12, pc}^
