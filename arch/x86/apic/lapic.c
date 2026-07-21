/*
 * Local APIC (Advanced Programmable Interrupt Controller) Driver
 */

#include <ABI/kernel/internal/kabi.h>

#define APIC_BASE_MSR 0x1B
#define APIC_SIVR     0x0F0
#define APIC_EOI      0x0B0
#define APIC_LVT_TIMER 0x320

void lapic_init(u64 lapic_base)
{
	/* Enable LAPIC by setting bit 8 in Spurious Interrupt Vector Register */
	volatile u32 *sivr = (volatile u32 *)(lapic_base + APIC_SIVR);
	*sivr |= 0x100 | 0xFF;
}

void lapic_eoi(u64 lapic_base)
{
	volatile u32 *eoi = (volatile u32 *)(lapic_base + APIC_EOI);
	*eoi = 0;
}
