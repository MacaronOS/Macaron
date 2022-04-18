MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

; Multiboot header
section .multiboot
dd  MBOOT_HEADER_MAGIC
dd  MBOOT_HEADER_FLAGS
dd  MBOOT_CHECKSUM

global start
extern boot_page_directory
extern init_boot_translation_table
start:
  ; Fill and set a boot page translation table.
  call init_boot_translation_table
  mov ecx, boot_page_directory
  mov cr3, ecx

  ; enable paging
  mov ecx, cr0
  or ecx, 0x80010000
  mov cr0, ecx

  ; jump to the kernel
  lea ecx, [L4]
  jmp ecx

section .text
extern kernel_entry_point
extern _kernel_stack_end
L4:
  mov     DWORD[boot_page_directory], 0
  mov     ecx, cr3
  mov     cr3, ecx
  mov     esp, _kernel_stack_end

  add ebx, 0xC0000000
  push ebx ; saving multiboot structure
  call kernel_entry_point
  jmp $