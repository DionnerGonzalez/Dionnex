; ============================================================================
; Dionnex Kernel - Process Context Switch Routine (arch/x86_64/switch.asm)
; ============================================================================

global switch_task

; switch_task(uint32_t* old_esp, uint32_t new_esp)
switch_task:
    push ebp
    mov ebp, esp

    ; Push registers to be preserved across C ABI
    push ebx
    push esi
    push edi

    ; Save current process stack pointer
    mov eax, [ebp + 8]  ; old_esp pointer
    mov [eax], esp

    ; Switch to new process stack pointer
    mov esp, [ebp + 12] ; new_esp value

    ; Restore preserved registers
    pop edi
    pop esi
    pop ebx

    pop ebp
    ret
