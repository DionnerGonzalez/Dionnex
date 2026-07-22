/* ============================================================================
 * Dionnex Real OS Kernel - x86 Linker Script (linker.ld)
 * ============================================================================
 */
ENTRY(_start)

SECTIONS
{
    /* Physical load address for x86 Protected Mode Kernel */
    . = 0x00100000; /* 1MB boundary */

    /* Multiboot header and code segment */
    .text ALIGN(0x1000) :
    {
        *(.multiboot)
        *(.text)
    }

    /* Read-only data segment */
    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    }

    /* Read-write data segment */
    .data ALIGN(0x1000) :
    {
        *(.data)
    }

    /* Uninitialized data segment (stack, BSS) */
    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}
