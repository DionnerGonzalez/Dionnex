; ============================================================================
; Dionnex Kernel - GDT Flush Assembly Routine (arch/x86_64/gdt_flush.asm)
; ============================================================================

global gdt_flush

gdt_flush:
    mov eax, [esp+4]  ; Get pointer to gdt_ptr
    lgdt [eax]

    mov ax, 0x10      ; 0x10 is offset in GDT to kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 is offset to kernel code segment (Far Jump)
.flush:
    ret
