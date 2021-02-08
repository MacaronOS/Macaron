[GLOBAL set_cr3]
[GLOBAL enable_paging]
[GLOBAL flush_cr3]
[GLOBAL get_cr2]

get_cr2:
    mov eax, cr2
    ret

set_cr3:
    mov eax, [esp+4]
    mov cr3, eax
    ret

enable_paging:
    cli
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    sti
    ret

flush_cr3:
    push eax
    mov eax, cr3
    mov cr3, eax
    pop eax
    ret