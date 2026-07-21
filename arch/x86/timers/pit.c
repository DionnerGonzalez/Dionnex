/*
 * Programmable Interval Timer (PIT 8254) Driver
 */

#include <ABI/kernel/internal/kabi.h>

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_BASE_HZ 1193182

void pit_init(u32 frequency_hz)
{
	u32 divisor = PIT_BASE_HZ / frequency_hz;
	__asm__ __volatile__("outb %0, %1" : : "a"((u8)0x36), "nd"((u16)PIT_COMMAND));
	__asm__ __volatile__("outb %0, %1" : : "a"((u8)(divisor & 0xFF)), "nd"((u16)PIT_CHANNEL0));
	__asm__ __volatile__("outb %0, %1" : : "a"((u8)((divisor >> 8) & 0xFF)), "nd"((u16)PIT_CHANNEL0));
}
