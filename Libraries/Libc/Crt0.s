global _start

extern main
extern _init

_start:
    call _init
	call main
	mov ebx, eax
	mov eax, 1
	int 0x80
