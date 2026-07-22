/* ============================================================================
 * Dionnex Kernel - Task State Segment TSS Setup (arch/x86_64/tss.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0; // Stack pointer for Kernel Mode transition
    uint32_t ss0;  // Stack segment for Kernel Mode transition
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct tss_entry tss;

void tss_set_kernel_stack(uint32_t kstack) {
    tss.esp0 = kstack;
}

void tss_init(void) {
    memset(&tss, 0, sizeof(struct tss_entry));
    tss.ss0 = 0x10; // Kernel Data Segment
    tss.esp0 = 0;   // Stack pointer updated dynamically
    
    // Flush Task Register (LTR)
    __asm__ __volatile__ ("ltr %%ax" : : "a"(0x2B)); // TSS selector
}
