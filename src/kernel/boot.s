MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

; locating stack
SECTION .bootstrap_stack nobits
stack_bottom:
resb 16384 ; 16 KiB
[GLOBAL stack_top]
stack_top:

; locating pages
SECTION .bss nobits
align 0x1000
[GLOBAL boot_page_directory]
boot_page_directory:
resb 4096
[GLOBAL boot_page_table1]
boot_page_table1:
resb 4096
[GLOBAL boot_page_table2]
boot_page_table2:
resb 4096

; kernel enty point
SECTION .multiboot
dd  MBOOT_HEADER_MAGIC
dd  MBOOT_HEADER_FLAGS
dd  MBOOT_CHECKSUM
[GLOBAL start]
[EXTERN _kernel_start]
[EXTERN _kernel_end]

start:
  mov edi, boot_page_table1 - 0xC0000000
  
  mov esi, 0 ; keeps phys addresses
  mov ecx, 1024 * 2

L1:
  mov edx, esi
  or edx, 0x007 ; present, writable
  mov DWORD[edi], edx

L2: 
  add esi, 4096
  add edi, 4
  loop L1

L3:
  ; page table 1
  mov ecx, boot_page_table1 - 0xC0000000
  or ecx, 0x007

  mov edx, boot_page_directory - 0xC0000000 + 0
  mov DWORD[edx], ecx ; identity

  mov edx, boot_page_directory - 0xC0000000 + 4 * 768 ; 0xC
  mov DWORD[edx], ecx ; higher half

  ; page table 2
  mov ecx, boot_page_table2 - 0xC0000000
  or ecx, 0x007
  
  add edx, 4
  mov DWORD[edx], ecx ; higher half

  ; set cr3
  mov ecx,  boot_page_directory - 0xC0000000
  mov cr3, ecx

  ; enable paging
  
  mov ecx, cr0
  or ecx, 0x80010000
  mov cr0, ecx

  ; jump to the kernel
  lea ecx, [L4]
  jmp ecx


SECTION .text

[EXTERN kernel_main]
[EXTERN call_constructors]

L4:
  mov     DWORD[boot_page_directory], 0
  mov     ecx, cr3
  mov     cr3, ecx
  mov     esp, stack_top

  call    call_constructors

  cli
  add ebx, 0xC0000000
  push ebx ; saving multiboot structure
  call kernel_main
  jmp $