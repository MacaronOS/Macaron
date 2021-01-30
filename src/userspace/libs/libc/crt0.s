global _start

extern main

_start:
	call main
	mov edi, eax
	mov eax, 1
	int 0x80
