; ============================================================================
; Dionnex Monolithic Kernel - Real Mode to Protected Mode AP Boot Trampoline
; Physical load address: 0x8000 (SIPI vector 0x08)
; Author: Ihosvanni Dionner Gonzalez
; ============================================================================

global ap_trampoline_start
global ap_trampoline_end

%define TRAMP_BASE 0x8000

section .rodata
align 16
ap_trampoline_start:
bits 16
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    lgdt [TRAMP_BASE + (ap_gdt_desc - ap_trampoline_start)]

    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp dword 0x08:(TRAMP_BASE + (ap_pm_entry - ap_trampoline_start))

bits 32
ap_pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Load BSP-patched page directory physical address and enable paging
    mov eax, [0x80F0]
    test eax, eax
    jz .skip_pg
    mov cr3, eax
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
.skip_pg:
    ; Load BSP-patched 32-bit stack and C entry function
    mov esp, [0x80F4]
    mov eax, [0x80F8]
    call eax

.halt_loop:
    cli
    hlt
    jmp .halt_loop

align 8
ap_gdt:
    dq 0x0000000000000000 ; Null
    dq 0x00CF9A000000FFFF ; Kernel Code 0x08
    dq 0x00CF92000000FFFF ; Kernel Data 0x10
ap_gdt_end:

ap_gdt_desc:
    dw ap_gdt_end - ap_gdt - 1
    dd TRAMP_BASE + (ap_gdt - ap_trampoline_start)

ap_trampoline_end:
