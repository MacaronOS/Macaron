// Boot process is described here:
// https://developer.arm.com/documentation/den0013/d/Boot-Code/Booting-a-bare-metal-system
// Note: while MacaronOS targets Cortex-A15, the example above is for Cortex-A9, but the
// boot process in nearly the same.

.section .boot, "ax"
.global start
start:
    b reset_handler
    b . /* 0x4  Undefined Instruction */
    b . /* 0x8  Software Interrupt */
    b . /* 0xC  Prefetch Abort */
    b . /* 0x10 Data Abort */
    b . /* 0x14 Reserved */
    ldr pc, irq_exception_handler_address
    b . /* 0x1C FIQ */

irq_exception_handler_address:
    .long irq_exception_handler

reset_handler:
    // Set the vector table base address.
    ldr r1, =start
    mcr p15, #0, r1, c12, c0, #0
    
    // Set a stack pointer for boot code.
    ldr r1, =_kernel_stack_end - (0xC0000000 - 0x80100000)
    mov sp, r1

    // Enable simplified access permission model.
    // https://developer.arm.com/documentation/ddi0406/b/System-Level-Architecture/Virtual-Memory-System-Architecture--VMSA-/Memory-access-control/Access-permissions?lang=en#BEIICBJD
    mrc p15, 0, r1, c1, c0, 0
    orr r1, r1, #0x4
    mcr p15, 0, r1, c1, c0, 0

    // Set all domains to client.
    ldr r1, =0x55555555
    mcr p15, 0, r1, c3, c0, 0

    // Initialize MMU.
    // Fill up a special translation table which's used during boot.
    bl init_boot_translation_table
    ldr r1, =boot_translation_table
    mcr p15, 0, r1, c2, c0, 0

    // Enable MMU.
    mrc p15, 0, r1, c1, c0, 0
    orr r1, r1, #0x1
    mcr p15, 0, r1, c1, c0, 0

    // Initialize stack pointers.
    ldr r1, =_svc_stack_end
    cps #0x13
    mov sp, r1

    ldr r1, =_irq_stack_end
    cps #0x12
    mov sp, r1

    ldr r1, =_abort_stack_end
    cps #0x17
    mov sp, r1

    ldr r1, =_undefined_stack_end
    cps #0x1b
    mov sp, r1

    ldr r1, =_kernel_stack_end
    cps #0x1f
    mov sp, r1

    // Write information about available memory.
    ldr r1, =emulate_multiboot
    blx r1
    
    // Jump to the c++ code.
    ldr r0, =multiboot_structure
    ldr r1, =kernel_entry_point
    blx r1
1:
    b 1b
