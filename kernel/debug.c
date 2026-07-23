/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Debug & Memory Diagnostic Implementation
 * ============================================================================
 */
#include <kernel/debug.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>

void debug_mem_stats(void) {
    uint32_t total_p = pmm_get_total_count();
    uint32_t free_p  = pmm_get_free_count();
    uint32_t used_p  = total_p - free_p;

    uint32_t heap_free_bytes = heap_get_free();
    uint32_t heap_used_bytes = heap_get_used();
    uint32_t heap_total = heap_free_bytes + heap_used_bytes;

    uint32_t frag_pct = (heap_total > 0) ? ((heap_free_bytes * 100) / heap_total) : 0;

    printk("\n=== MEMORY DIAGNOSTIC STATS ===\n");
    printk("PHYSICAL MEMORY:\n");
    printk("  Total Pages : %u (%u MB)\n", total_p, (total_p * 4) / 1024);
    printk("  Used Pages  : %u (%u MB)\n", used_p, (used_p * 4) / 1024);
    printk("  Free Pages  : %u (%u MB)\n", free_p, (free_p * 4) / 1024);
    printk("KERNEL HEAP:\n");
    printk("  Total Heap  : %u KB\n", heap_total / 1024);
    printk("  Used Heap   : %u KB\n", heap_used_bytes / 1024);
    printk("  Free Heap   : %u KB\n", heap_free_bytes / 1024);
    printk("  Free Ratio  : %u%%\n", frag_pct);
    printk("================================\n\n");
}

void debug_dump_stack(uint32_t count) {
    uint32_t *ebp;
    asm volatile ("movl %%ebp, %0" : "=r"(ebp));

    printk("--- STACK BACKTRACE ---\n");
    for (uint32_t i = 0; i < count && ebp; i++) {
        uint32_t eip = ebp[1];
        if (eip == 0) break;
        printk("[%u] EBP: 0x%08x  EIP: 0x%08x\n", i, (uint32_t)ebp, eip);
        ebp = (uint32_t*)ebp[0];
    }
    printk("-----------------------\n");
}

void debug_hexdump(const void *addr, uint32_t len) {
    if (!addr || len == 0) return;

    const uint8_t *ptr = (const uint8_t*)addr;
    for (uint32_t i = 0; i < len; i += 16) {
        printk("0x%08x  ", (uint32_t)ptr + i);

        // Hex representation
        for (uint32_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printk("%02x ", ptr[i + j]);
            } else {
                printk("   ");
            }
        }

        printk(" |");

        // ASCII representation
        for (uint32_t j = 0; j < 16; j++) {
            if (i + j < len) {
                uint8_t c = ptr[i + j];
                if (c >= 32 && c <= 126) {
                    printk("%c", c);
                } else {
                    printk(".");
                }
            }
        }
        printk("|\n");
    }
}

void debug_page_table_info(void) {
    page_directory_t *dir = vmm_get_kernel_directory();
    if (!dir) return;

    uint32_t mapped_tables = 0;
    for (int i = 0; i < 1024; i++) {
        if (dir->entries[i] & VMM_PRESENT) {
            mapped_tables++;
        }
    }

    printk("VMM: Kernel Page Directory at 0x%x (%u active page tables)\n",
           (uint32_t)dir, mapped_tables);
}
