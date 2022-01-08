global loader

; Some constants

MODULEALIGN equ 1<<0
MEMINFO     equ 1<<1
MAGIC       equ 0x1BADB002
FLAGS       equ MODULEALIGN | MEMINFO
CHECKSUM    equ -(MAGIC + FLAGS)    ; magic + checksum + flags = 0


KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_PAGE_NUMBER  equ (KERNEL_VIRTUAL_BASE >> 22)
KERNEL_PDE_INDEX equ (KERNEL_VIRTUAL_BASE >> 22)


section .data                       ; data section
align 4
boot_page_directory:
    times 1024 dd 2
boot_page_table:
    times 1024 dd 0


section .multiboot                  ; multiboot header
align 4

    dd MAGIC
    dd FLAGS
    dd CHECKSUM


section .bss                        ; bss section
align 4

stack_bottom:
    resb 0xE000                  ; 0x1200*n
stack_top:                          ; the stack
    resb 0x1200

section .text                       ; code section
align 4

; Entry point for ELF
loader:

    ; All addresses are set by linker in virtual address space
    ; Subtract by VIRTUAL_BASE to get physical address before we enable paging


    mov ecx, (stack_top - KERNEL_VIRTUAL_BASE)
    mov esp, ecx
    push ebx

    ; Fill the page table
    mov ebx, (boot_page_table - KERNEL_VIRTUAL_BASE)
    mov ecx, 1024
    mov edx, 0
    mov edi, 0
L1: mov dword[ebx+edi], edx
    or dword[ebx+edi], 3
    add edi, 4
    add edx, 4096
    loop L1

    ; Fill the page directory
    ; First 4 MB identity mapped
    mov ebx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov dword[ebx+0], (boot_page_table - KERNEL_VIRTUAL_BASE)
    or dword[ebx+0], 3
    ; Higher half kernel mapping    
    mov dword[ebx+KERNEL_PDE_INDEX*4], (boot_page_table - KERNEL_VIRTUAL_BASE)
    or dword[ebx+KERNEL_PDE_INDEX*4], 3

    pop ebx

    ; Load the page directory and enable paging
    mov ecx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, [higher_half_start]
    jmp ecx                         ; long absolute jump

higher_half_start:
    mov esp, stack_top
    ; call kernel main function
    ;add esp,0xC0000000
    ; pass the multiboot header address, which GRUB stores at ebx
    add ebx, KERNEL_VIRTUAL_BASE
    push ebx

    extern kernel_main
    call kernel_main

    cli
    hlt

.hang:
    jmp .hang