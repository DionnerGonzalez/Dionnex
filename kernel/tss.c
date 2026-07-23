/* ============================================================================
 * Dionnex Monolithic Kernel - Task State Segment (TSS) Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/tss.h>
#include <kernel/gdt.h>
#include <kernel/printk.h>
#include <string.h>

static tss_entry_t tss;

void tss_set_kernel_stack(uint32_t esp0) {
    tss.esp0 = esp0;
}

void tss_init(void) {
    memset(&tss, 0, sizeof(tss_entry_t));

    tss.ss0 = 0x10; // Kernel Data Segment
    tss.esp0 = 0;   // Will be set per user process
    tss.iomap_base = sizeof(tss_entry_t);

    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss_entry_t) - 1;

    // GDT entry 5: TSS Descriptor (0x28)
    // access = 0xE9 (Present, Ring 3 executable/accessible, Executable|Accessed => 32-bit TSS Available)
    gdt_set_gate(5, base, limit, 0xE9, 0x00);

    tss_flush();

    printk("TSS: Task State Segment initialized at 0x%x\n", (uint32_t)&tss);
}
