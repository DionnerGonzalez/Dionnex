/* ============================================================================
 * Dionnex Monolithic Kernel - Advanced Programmable Interrupt Controller (APIC)
 * Adapted from Linux kernel (GPLv2) - arch/x86/kernel/apic/
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_APIC_H_
#define _DIONNEX_APIC_H_

#include <types.h>

#define LAPIC_BASE_DEFAULT   0xFEE00000

#define LAPIC_ID             0x20
#define LAPIC_VERSION        0x30
#define LAPIC_TPR            0x80
#define LAPIC_EOI            0xB0
#define LAPIC_SVR            0xF0
#define LAPIC_ESR            0x280
#define LAPIC_ICR_LOW        0x300
#define LAPIC_ICR_HIGH       0x310
#define LAPIC_TIMER          0x320
#define LAPIC_THERMAL        0x330
#define LAPIC_ERROR          0x370
#define LAPIC_TIMER_INIT     0x380
#define LAPIC_TIMER_CURRENT  0x390
#define LAPIC_TIMER_DIVIDE   0x3E0

#define IOAPIC_BASE_DEFAULT  0xFEC00000

#define IOAPIC_REG_ID        0x00
#define IOAPIC_REG_VER       0x01
#define IOAPIC_REG_TABLE     0x10

void apic_init(void);
void apic_eoi(void);
uint32_t apic_read(uint32_t reg);
void apic_write(uint32_t reg, uint32_t val);
uint32_t ioapic_read(uint32_t reg);
void ioapic_write(uint32_t reg, uint32_t val);
void ioapic_route_irq(uint8_t irq, uint8_t vector, uint8_t dest);
void apic_timer_init(uint32_t frequency);
void apic_disable_pic(void);

#endif /* _DIONNEX_APIC_H_ */
