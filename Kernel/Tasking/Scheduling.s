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

[GLOBAL block_and_switch_to_kernel]
block_and_switch_to_kernel:
    mov eax, [esp + 4] ; KernelContext** cur
    mov edx, [esp + 8] ; KernelContext** next

    push edi
    push esi
    push ebp
    push ebx

    mov [eax], esp
    mov esp, [edx]
    mov DWORD[edx], 0 ; so the next thread no longer thinks it's blocked

    pop ebx
    pop ebp
    pop esi
    pop edi

    ret

[GLOBAL block_and_switch_to_user]
block_and_switch_to_user:
    mov eax, [esp + 4] ; KernelContext** cur
    mov edx, [esp + 8] ; Trapframe* next

    push edi
    push esi
    push ebp
    push ebx

    mov [eax], esp
    mov esp, edx

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