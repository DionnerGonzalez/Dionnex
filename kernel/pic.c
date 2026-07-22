/* ============================================================================
 * Dionnex Kernel - 8259 PIC Controller Remap (kernel/pic.c)
 * ============================================================================
 */
#include <kernel.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

void pic_remap(void) {
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();

    outb(PIC1_DATA, 0x20); io_wait(); // Master PIC vector offset 32 (0x20)
    outb(PIC2_DATA, 0x28); io_wait(); // Slave PIC vector offset 40 (0x28)

    outb(PIC1_DATA, 0x04); io_wait(); // Tell Master PIC slave at IRQ2
    outb(PIC2_DATA, 0x02); io_wait(); // Tell Slave PIC cascade identity

    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    /* Restore masks */
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);

    vga_puts("[PIC] 8259 PIC remapeado a vectores 0x20 (IRQ0-7) y 0x28 (IRQ8-15).\n");
}
