; ============================================================================
; Dionnex Monolithic Kernel - Multiboot Assembly Entry Point
; ============================================================================

MBALIGN     equ 1 << 0
MEMINFO     equ 1 << 1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB Stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top

    ; Push multiboot information pointer and magic number
    push ebx
    push eax

    ; Call main C kernel entry point
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
