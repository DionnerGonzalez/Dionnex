/* ============================================================================
 * Dionnex Kernel - Programmable Interval Timer Driver (kernel/timer.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

static uint64_t timer_ticks = 0;
static uint32_t timer_frequency = 100;

void timer_irq_handler(void) {
    timer_ticks++;
    if (timer_ticks % 100 == 0) {
        // Yield to scheduler every 1 second or on slice exhaustion
        schedule();
    }
}

void pit_init(uint32_t frequency) {
    timer_frequency = frequency;
    uint32_t divisor = 1193180 / frequency;

    // Send Command Byte: Channel 0, Access mode lobyte/hibyte, Mode 2 (rate generator)
    outb(0x43, 0x36);

    // Divisor low and high bytes
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);

    // Register with IRQ0
    irq_register_handler(0, timer_irq_handler);
}

uint64_t pit_get_ticks(void) {
    return timer_ticks;
}

void ksleep(uint32_t ms) {
    uint64_t target_ticks = timer_ticks + (ms * timer_frequency) / 1000;
    while (timer_ticks < target_ticks) {
        hlt();
    }
}
