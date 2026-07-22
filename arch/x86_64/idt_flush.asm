; ============================================================================
; Dionnex Kernel - IDT Flush Assembly Routine (arch/x86_64/idt_flush.asm)
; ============================================================================

global idt_flush

idt_flush:
    mov eax, [esp+4]  ; Get pointer to idt_ptr passed as parameter
    lidt [eax]        ; Load IDT
    ret
