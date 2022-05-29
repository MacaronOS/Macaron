[GLOBAL gdt_flush]
[GLOBAL idt_flush]
[GLOBAL tss_flush]

idt_flush:
   mov eax, [esp+4]
   lidt [eax]
   ret

gdt_flush:
   mov eax, [esp+4]
   lgdt [eax]

   mov ax, 0x10
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   jmp 0x08:.flush
.flush:
   ret

tss_flush:
   mov ax, 0x2b
   ltr ax
   ret