[GLOBAL return_from_scheduler]
return_from_scheduler:
    mov esp, [esp+4]

    pop eax
    mov ds, ax

    pop eax
    mov es, ax

    pop eax
    mov fs, ax

    pop eax
    mov gs, ax

    popad
    add esp, 8
    iretd

[GLOBAL return_to_the_kernel_handler]
return_to_the_kernel_handler:
    mov esp, [esp+4]
    pop edi
    pop esi
    pop ebx
    pop ebp
    pop esp
    ret

[GLOBAL switch_to_user_mode]
switch_to_user_mode:
    cli
    mov ax, 0x20 | 0x3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push 0x20 | 0x3
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x18 | 0x3
    lea eax, [L1]
    push eax
    iretd
L1:
    ret