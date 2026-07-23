; ============================================================================
; Dionnex Monolithic Kernel - Task Context Switch Assembly
; Author: Ihosvanni Dionner Gonzalez
; ============================================================================

global context_switch
global context_switch_user

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

; void context_switch_user(uint32_t *old_esp, uint32_t new_esp, uint32_t new_cr3)
context_switch_user:
    pusha
    pushf

    mov eax, [esp + 36]  ; old_esp argument pointer
    cmp eax, 0
    je .no_save
    mov [eax], esp       ; Save current stack pointer

.no_save:
    mov edx, [esp + 44]  ; new_cr3
    cmp edx, 0
    je .no_cr3
    mov cr3, edx         ; Load new page directory

.no_cr3:
    mov esp, [esp + 40]  ; new_esp argument value

    popf
    popa
    ret
