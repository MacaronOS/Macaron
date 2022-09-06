; void block_and_switch_to_kernel(KernelContext** cur, KernelContext** next)
global block_and_switch_to_kernel
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

    jmp switch_to_kernel_out

; void switch_to_kernel(KernelContext** kc)
global switch_to_kernel
switch_to_kernel:
    mov eax, [esp + 4] ; KernelContext** kc

    mov esp, [eax]
    mov DWORD[eax], 0 ; so the thread no longer thinks it's blocked

switch_to_kernel_out:
    pop ebx
    pop ebp
    pop esi
    pop edi

    ret

; void block_and_switch_to_user(KernelContext** cur, Trapframe* next)
global block_and_switch_to_user
block_and_switch_to_user:
    mov eax, [esp + 4] ; KernelContext** cur
    mov edx, [esp + 8] ; Trapframe* next

    push edi
    push esi
    push ebp
    push ebx

    mov [eax], esp
    mov esp, edx

    jmp switch_to_user_out

; void switch_to_user(Trapframe* tf)
global switch_to_user
switch_to_user:
    mov esp, [esp+4] ; Trapframe* tf

switch_to_user_out:
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
