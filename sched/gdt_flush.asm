; ============================================================================
; Dionnex Monolithic Kernel - GDT Flush Routine
; ============================================================================

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Pointer to gdt_ptr
    lgdt [eax]

    mov ax, 0x10        ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush     ; Far jump to reload CS register

.flush:
    ret
