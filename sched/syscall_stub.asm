; ============================================================================
; Dionnex Monolithic Kernel - Syscall Assembly Entry Point Stub (INT 0x80)
; Author: Ihosvanni Dionner Gonzalez
; ============================================================================

global syscall_isr
extern syscall_handler

syscall_isr:
    cli
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call syscall_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret
