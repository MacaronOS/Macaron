ENTRY(start)
 
SECTIONS {
    . = 0x80100000;
    _boot_start = .;
    .boot   : { *(.boot.*) }
    .boot_init_bss ALIGN(4K): {
        *(.boot_init_bss.*)
    }
    .boot_init_text ALIGN(4K): {
        *(.boot_init_text.*)
    }
    _boot_end = .;

    . += (0xC0000000 - 0x80100000);

    _kernel_start = .;
    _kernel_text_start = .;

    .text ALIGN(4K) : AT(ADDR(.text) - (0xC0000000 - 0x80100000))
    {
        *(.text*)
    }

    _kernel_text_end = .;
    _kernel_rodata_start = .;

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - (0xC0000000 - 0x80100000))
    {
        *(.rodata*)
    }

    _kernel_rodata_end = .;
    _kernel_data_start = .;
 
    .data ALIGN(4K) : AT(ADDR(.data) - (0xC0000000 - 0x80100000))
    {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        KEEP(*(.ctor*));
        end_ctors = .;

        data = .;
        *(.data*)
    }

    _kernel_data_end = .;
    _kernel_bss_start = .;

    .bss ALIGN(4K) : AT(ADDR(.bss) - (0xC0000000 - 0x80100000))
    {
        bss = .;
        *(COMMON)
        *(.bss*)
    }

    .stacks ALIGN(4K) : AT(ADDR(.stacks) - (0xC0000000 - 0x80100000))
    {
        _kernel_stack_start = .;
        . += 0x10000;
        _kernel_stack_end = .;

        _svc_stack_start = .;
		. += 0x1000;
		_svc_stack_end =  .;

		_irq_stack_start = .;
		. += 0x1000;
		_irq_stack_end =  .;

		_abort_stack_start = .;
		. += 0x1000;
		_abort_stack_end =  .;

		_undefined_stack_start = .;
		. += 0x1000;
		_undefined_stack_end =  .;
    }

    _kernel_bss_end = .;
    . = ALIGN(4K);
    _kernel_end = .;

    _end = .;

}