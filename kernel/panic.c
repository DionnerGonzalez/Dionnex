/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Panic Handling Implementation
 * ============================================================================
 */
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <drivers/vga.h>

void kernel_panic(const char *msg) {
    asm volatile("cli");
    vga_set_color(0x0F, 0x04); // White text on Red background
    vga_clear();

    vga_write("\n\n  ========================================\n");
    vga_write("  KERNEL PANIC\n");
    vga_write("  ========================================\n\n  ");
    vga_write(msg ? msg : "Unspecified critical error");
    vga_write("\n\n  System halted. Reboot to continue.\n");

    for (;;) {
        asm volatile("hlt");
    }
}

void kernel_assert(int condition, const char *file, int line, const char *msg) {
    if (!condition) {
        printk("Assertion failed: %s at %s:%d\n", msg ? msg : "condition", file ? file : "unknown", line);
        kernel_panic("Assertion failure");
    }
}
