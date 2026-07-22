/* ============================================================================
 * Dionnex Kernel - 8253 PIT Timer & IRQ0 Handler (kernel/pit.c)
 * ============================================================================
 */
#include <kernel.h>

static uint64_t pit_ticks = 0;

uint64_t pit_get_ticks(void) {
    return pit_ticks;
}

void pit_handler(void) {
    pit_ticks++;
    // Send EOI to PIC Master
    outb(0x20, 0x20);
}

void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    // Command byte: Channel 0, Access mode lobyte/hibyte, Square wave generator
    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);

    vga_puts("[PIT] Temporizador 8253 PIT configurado a 100Hz (IRQ0 activo).\n");
}
