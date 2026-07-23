/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Panic & Assertions Implementation
 * ============================================================================
 */
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <kernel/klog.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <kernel/debug.h>

void kernel_panic(const char *msg) {
    asm volatile ("cli");

    // Red background on VGA
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();

    printk("\n===================================================================\n");
    printk("                    !!! KERNEL PANIC !!!                            \n");
    printk("===================================================================\n\n");

    if (msg) {
        printk("REASON: %s\n\n", msg);
        klog(KLOG_PANIC, "PANIC: %s\n", msg);
    } else {
        printk("REASON: Unknown Fatal Error\n\n");
    }

    // Dump register state
    uint32_t eax, ebx, ecx, edx, esp, ebp;
    asm volatile ("movl %%eax, %0; movl %%ebx, %1; movl %%ecx, %2; movl %%edx, %3;"
                  "movl %%esp, %4; movl %%ebp, %5;"
                  : "=r"(eax), "=r"(ebx), "=r"(ecx), "=r"(edx), "=r"(esp), "=r"(ebp));

    printk("REGISTERS:\n");
    printk("  EAX: 0x%08x  EBX: 0x%08x  ECX: 0x%08x  EDX: 0x%08x\n", eax, ebx, ecx, edx);
    printk("  ESP: 0x%08x  EBP: 0x%08x\n\n", esp, ebp);

    // Dump stack trace
    debug_dump_stack(10);

    printk("\nSystem halted. Please reboot the machine.\n");

    while (1) {
        asm volatile ("hlt");
    }
}

void kernel_assert(int condition, const char *file, int line, const char *expr) {
    if (!condition) {
        char err_buf[256];
        err_buf[0] = '\0';
        printk("Assertion failed: (%s) at %s:%d\n", expr ? expr : "false", file ? file : "unknown", line);
        kernel_panic("Kernel Assertion Failed");
    }
}
