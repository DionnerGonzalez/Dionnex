; ============================================================================
; Dionnex Kernel - Multiboot Assembly Entry Point (arch/x86_64/boot.asm)
; ============================================================================

MBALIGN     equ  1 << 0
MEMINFO     equ  1 << 1
FLAGS       equ  MBALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB Kernel Stack
stack_top:

section .text
global _start
extern start_kernel

_start:
    cli
    mov esp, stack_top

    ; Call main C Kernel Entry Point
    call start_kernel

.halt:
    cli
    hlt
    jmp .halt
