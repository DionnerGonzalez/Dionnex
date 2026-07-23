/* ============================================================================
 * Dionnex Monolithic Kernel - Advanced Programmable Interrupt Controller (APIC)
 * Adapted from Linux kernel (GPLv2) - arch/x86/kernel/apic/
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/apic.h>
#include <kernel/cpuid.h>
#include <kernel/printk.h>
#include <mm/vmm.h>
#include <io.h>

static volatile uint32_t *lapic_base = (volatile uint32_t*)LAPIC_BASE_DEFAULT;
static volatile uint32_t *ioapic_base = (volatile uint32_t*)IOAPIC_BASE_DEFAULT;

uint32_t apic_read(uint32_t reg) {
    return *(volatile uint32_t*)((uintptr_t)lapic_base + reg);
}

void apic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)((uintptr_t)lapic_base + reg) = val;
}

uint32_t ioapic_read(uint32_t reg) {
    volatile uint32_t *iowin = (volatile uint32_t*)((uintptr_t)ioapic_base + 0x00);
    volatile uint32_t *iodat = (volatile uint32_t*)((uintptr_t)ioapic_base + 0x10);
    *iowin = reg;
    return *iodat;
}

void ioapic_write(uint32_t reg, uint32_t val) {
    volatile uint32_t *iowin = (volatile uint32_t*)((uintptr_t)ioapic_base + 0x00);
    volatile uint32_t *iodat = (volatile uint32_t*)((uintptr_t)ioapic_base + 0x10);
    *iowin = reg;
    *iodat = val;
}

void ioapic_route_irq(uint8_t irq, uint8_t vector, uint8_t dest) {
    uint32_t reg_low = IOAPIC_REG_TABLE + (irq * 2);
    uint32_t reg_high = IOAPIC_REG_TABLE + (irq * 2) + 1;

    ioapic_write(reg_high, (uint32_t)dest << 24);
    ioapic_write(reg_low, (uint32_t)vector);
}

void apic_disable_pic(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void apic_eoi(void) {
    apic_write(LAPIC_EOI, 0);
}

void apic_timer_init(uint32_t frequency) {
    (void)frequency;
    apic_write(LAPIC_TIMER_DIVIDE, 0x03); // Divide by 16
    apic_write(LAPIC_TIMER, 32 | 0x20000); // Periodic mode, IRQ0 / vector 32
    apic_write(LAPIC_TIMER_INIT, 1000000); // Initial counter
}

void apic_init(void) {
    uint64_t msr = rdmsr(MSR_IA32_APICBASE);
    if (!(msr & (1 << 11))) {
        msr |= (1 << 11);
        wrmsr(MSR_IA32_APICBASE, msr);
    }

    uint32_t base_addr = (uint32_t)(msr & 0xFFFFF000);
    if (base_addr != 0) {
        lapic_base = (volatile uint32_t*)base_addr;
    }

    vmm_map_page((void*)lapic_base, (void*)lapic_base, VMM_PRESENT | VMM_WRITABLE);
    vmm_map_page((void*)ioapic_base, (void*)ioapic_base, VMM_PRESENT | VMM_WRITABLE);

    apic_disable_pic();

    // Enable Local APIC
    apic_write(LAPIC_SVR, 0x1FF);

    // Route legacy IRQs 0-15 to vectors 32-47
    for (uint8_t i = 0; i < 16; i++) {
        ioapic_route_irq(i, 32 + i, 0);
    }

    apic_timer_init(1000);

    uint32_t version = ioapic_read(IOAPIC_REG_VER);
    printk("APIC: Local APIC at 0x%x, ID %u\n", (uint32_t)lapic_base, apic_read(LAPIC_ID) >> 24);
    printk("APIC: I/O APIC at 0x%x, version %u, %u redirection entries\n",
           (uint32_t)ioapic_base, version & 0xFF, (version >> 16) & 0xFF);
}
