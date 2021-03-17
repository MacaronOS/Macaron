[GLOBAL sys_printd]
sys_printd:
    mov eax, 4
    mov ebx, DWORD[esp+4]
    int 0x80
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