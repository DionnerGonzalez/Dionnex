; ============================================================================
; Dionnex Monolithic Kernel - Task Context Switch Assembly
; ============================================================================

global context_switch

; void context_switch(uint32_t *old_esp, uint32_t new_esp)
context_switch:
    pusha
    pushf

    mov eax, [esp + 36]  ; old_esp argument pointer
    mov [eax], esp       ; Save current stack pointer to old task structure

    mov esp, [esp + 40]  ; new_esp argument value (Switch stack to new task)

    popf
    popa
    ret
