/* ============================================================================
 * Dionnex Kernel - Kernel Panic Subsystem (kernel/panic.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

void panic(const char* message) {
    cli(); // Disable interrupts immediately
    
    vga_set_color(0x0F, 0x04); // White text on Red background
    vga_clear();

    kprintf("========================================================\n");
    kprintf("                 KERNEL PANIC DETECTED                  \n");
    kprintf("========================================================\n\n");
    kprintf("A critical unrecoverable exception occurred in kernel space:\n\n");
    kprintf("   -> REASON: %s\n\n", message ? message : "Unknown System Fault");
    kprintf("System halted. Please restart the virtual machine.\n");

    while (1) {
        hlt();
    }
}
