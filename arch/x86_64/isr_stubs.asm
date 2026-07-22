; ============================================================================
; Dionnex Kernel - ISR & IRQ Interrupt Vector Stubs (arch/x86_64/isr_stubs.asm)
; ============================================================================

extern irq_handler

global irq0_stub
global irq1_stub

; Common IRQ Handler assembly trampoline
irq_common_stub:
    pusha           ; Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
    mov ax, ds
    push eax        ; Save data segment descriptor

    mov ax, 0x10    ; Load kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp        ; Pass pointer to cpu_registers struct to C irq_handler
    call irq_handler
    add esp, 4

    pop eax         ; Restore original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Restore general registers
    add esp, 8      ; Clean up error code and int_no
    iret            ; Interrupt Return

irq0_stub:
    push dword 0    ; Dummy error code
    push dword 32   ; IRQ0 -> Vector 32
    jmp irq_common_stub

irq1_stub:
    push dword 0    ; Dummy error code
    push dword 33   ; IRQ1 -> Vector 33
    jmp irq_common_stub
