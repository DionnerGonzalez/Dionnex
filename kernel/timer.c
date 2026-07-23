/* ============================================================================
 * Dionnex Monolithic Kernel - Programmable Interval Timer (PIT 8253/8254)
 * ============================================================================
 */
#include <kernel/timer.h>
#include <kernel/idt.h>
#include <kernel/printk.h>
#include <drivers/pic.h>
#include <io.h>

#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43
#define PIT_FREQUENCY 1193182

static uint32_t timer_ticks = 0;
static uint32_t timer_freq = 1000;

extern void schedule(void);

static void timer_handler(registers_t *regs) {
    (void)regs;
    timer_ticks++;

    if (timer_ticks % 10 == 0) {
        schedule();
    }
}

void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    uint32_t divisor = PIT_FREQUENCY / frequency;

    outb(PIT_CMD, 0x36); // Channel 0, lobyte/hibyte, mode 3 (square wave generator)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    isr_register_handler(32, timer_handler);
    pic_enable_irq(0);

    printk("Timer: PIT initialized at %u Hz (divisor: %u)\n", frequency, divisor);
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = timer_ticks + (ms * timer_freq / 1000);
    while (timer_ticks < target) {
        asm volatile("hlt");
    }
}
