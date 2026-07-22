; ============================================================================
; Dionnex Kernel - x86 Context Switch Assembly Routine (arch/x86/switch.asm)
; ============================================================================
global switch_to

section .text

; void switch_to(uint32_t** old_esp, uint32_t* new_esp);
switch_to:
    push ebp
    mov ebp, esp

    ; Save callee-saved general registers
    push ebx
    push esi
    push edi

    ; eax = &prev->esp
    mov eax, [ebp + 8]
    ; edx = next->esp
    mov edx, [ebp + 12]

    ; Store old ESP
    mov [eax], esp

    ; Switch stack pointer to new task stack
    mov esp, edx

    ; Restore callee-saved registers of new task
    pop edi
    pop esi
    pop ebx

    pop ebp
    ret
