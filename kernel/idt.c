/* ============================================================================
 * Dionnex Kernel - Interrupt Descriptor Table (kernel/idt.c)
 * ============================================================================
 */
#include <kernel.h>

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt_entries[256];
static struct idt_ptr   idt_p;

extern void isr_stub_default(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

void idt_init(void) {
    idt_p.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_p.base  = (uint32_t)&idt_entries;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_default, 0x08, 0x8E); // Kernel Ring 0 Interrupt Gate
    }

    __asm__ __volatile__("lidt %0" : : "m"(idt_p));

    vga_puts("[IDT] IDT configurada con 256 vectores de interrupcion.\n");
}

void isr_default_handler(void) {
    // Basic handler for unhandled IRQ/exception
}

void isr_stub_default(void) {
    __asm__ __volatile__(
        "pusha\n"
        "call isr_default_handler\n"
        "popa\n"
        "iret\n"
    );
}
