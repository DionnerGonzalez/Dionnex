; ============================================================================
; Dionnex Monolithic Kernel - TSS Selector Load Assembly
; Author: Ihosvanni Dionner Gonzalez
; ============================================================================

global tss_flush

tss_flush:
    mov ax, 0x28  ; TSS Selector in GDT (index 5)
    ltr ax        ; Load Task Register
    ret
