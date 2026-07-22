; ============================================================================
; Dionnex Real OS Kernel - Multiboot Assembly Header & Entry Point
; ============================================================================
MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB Stack for Kernel boot
stack_top:

section .text
global _start:function (_start.end - _start)
extern kernel_main

_start:
    cli                     ; Disable interrupts early
    mov esp, stack_top      ; Set up kernel stack

    ; Push multiboot parameters to C kernel_main(magic, multiboot_addr)
    push ebx                ; Pointer to Multiboot structure
    push eax                ; Multiboot Magic Number (0x2BADB002)

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
.end:
