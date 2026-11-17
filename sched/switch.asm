; ============================================================================
; Dionnex Monolithic Kernel - Task & Ring 3 Context Switch Assembly
; Author: Ihosvanni Dionner Gonzalez
; ============================================================================

global context_switch
global context_switch_user
global ring3_trampoline_enter
global ring3_trampoline_exit

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
    mov [eax], esp

.no_save:
    mov edx, [esp + 44]  ; new_cr3
    cmp edx, 0
    je .no_cr3
    mov cr3, edx

.no_cr3:
    mov esp, [esp + 40]

    popf
    popa
    ret

; void ring3_trampoline_enter(uint32_t *saved_kesp, uint32_t eip, uint32_t user_esp, uint32_t cr3)
ring3_trampoline_enter:
    push ebp
    push ebx
    push esi
    push edi
    pushf

    mov eax, [esp + 24]  ; saved_kesp
    mov [eax], esp

    mov ebx, [esp + 28]  ; user eip
    mov ecx, [esp + 32]  ; user esp
    mov edx, [esp + 36]  ; new cr3

    cli
    cmp edx, 0
    je .skip_cr3
    mov cr3, edx
.skip_cr3:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword 0x23      ; SS
    push ecx             ; User ESP
    pushfd
    pop eax
    or eax, 0x200        ; Enable IF in Ring 3
    push eax             ; EFLAGS
    push dword 0x1B      ; User CS
    push ebx             ; User EIP
    iret

; void ring3_trampoline_exit(uint32_t saved_kesp, uint32_t kernel_cr3)
ring3_trampoline_exit:
    mov eax, [esp + 4]   ; saved_kesp
    mov edx, [esp + 8]   ; kernel_cr3

    cli
    mov esp, eax
    cmp edx, 0
    je .no_kcr3
    mov cr3, edx
.no_kcr3:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popf
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
