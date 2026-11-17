/* ============================================================================
 * Dionnex Monolithic Kernel - Advanced Programmable Interrupt Controller (APIC)
 * Adapted from Linux kernel (GPLv2) - arch/x86/kernel/apic/
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_APIC_H_
#define _DIONNEX_APIC_H_

#include <types.h>

#define LAPIC_BASE_DEFAULT   0xFEE00000U

#define LAPIC_ID             0x020U
#define LAPIC_VERSION        0x030U
#define LAPIC_TPR            0x080U
#define LAPIC_EOI            0x0B0U
#define LAPIC_SVR            0x0F0U
#define LAPIC_ESR            0x280U
#define LAPIC_ICR_LOW        0x300U
#define LAPIC_ICR_HIGH       0x310U
#define LAPIC_TIMER          0x320U
#define LAPIC_THERMAL        0x330U
#define LAPIC_ERROR          0x370U
#define LAPIC_TIMER_INIT     0x380U
#define LAPIC_TIMER_CURRENT  0x390U
#define LAPIC_TIMER_DIVIDE   0x3E0U

#define IOAPIC_BASE_DEFAULT  0xFEC00000U

#define IOAPIC_REG_ID        0x00U
#define IOAPIC_REG_VER       0x01U
#define IOAPIC_REG_TABLE     0x10U

void apic_init(void);
int apic_is_enabled(void);
void apic_eoi(void);
uint32_t apic_get_id(void);
uint32_t apic_read(uint32_t reg);
void apic_write(uint32_t reg, uint32_t val);
uint32_t ioapic_read(uint32_t reg);
void ioapic_write(uint32_t reg, uint32_t val);
void ioapic_route_irq(uint8_t irq, uint8_t vector, uint8_t dest);
void apic_timer_init(uint32_t frequency);
void apic_disable_pic(void);
void apic_send_init_ipi(uint8_t target_apic_id);
void apic_send_sipi_ipi(uint8_t target_apic_id, uint8_t vector_page);

#endif /* _DIONNEX_APIC_H_ */
