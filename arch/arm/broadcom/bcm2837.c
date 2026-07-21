/*
 * Broadcom BCM2837 / Raspberry Pi 3 ARM SoC Driver
 */

#include <ABI/kernel/internal/kabi.h>

#define BCM2837_PERI_BASE 0x3F000000
#define UART0_DR          (BCM2837_PERI_BASE + 0x201000)

void bcm2837_putc(char c)
{
	volatile u32 *uart = (volatile u32 *)UART0_DR;
	*uart = (u32)c;
}
