/* ============================================================================
 * Dionnex Kernel - Interrupt Request Handler Controller (drivers/irq.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

static void (*irq_routines[16])(void);

void irq_register_handler(int irq, void (*handler)(void)) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = handler;
    }
}

void irq_init(void) {
    for (int i = 0; i < 16; i++) {
        irq_routines[i] = NULL;
    }
}

void irq_handler(struct cpu_registers* regs) {
    int irq = regs->int_no - 32;

    if (irq >= 0 && irq < 16 && irq_routines[irq] != NULL) {
        irq_routines[irq]();
    }

    // Send End of Interrupt (EOI) signal to 8259 PIC controllers
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}
