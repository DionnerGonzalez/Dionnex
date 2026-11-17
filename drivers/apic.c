/* ============================================================================
 * Dionnex Monolithic Kernel - Advanced Programmable Interrupt Controller (APIC)
 * Adapted from Linux kernel (GPLv2) - arch/x86/kernel/apic/
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/apic.h>
#include <kernel/cpuid.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <io.h>

static volatile uint32_t *lapic_virt = NULL;
static volatile uint32_t *ioapic_virt = NULL;
static uint32_t lapic_phys = LAPIC_BASE_DEFAULT;
static uint32_t ioapic_phys = IOAPIC_BASE_DEFAULT;
static int apic_ready = 0;

uint32_t apic_read(uint32_t reg) {
    if (!lapic_virt) return 0;
    return *(volatile uint32_t*)((uintptr_t)lapic_virt + reg);
}

void apic_write(uint32_t reg, uint32_t val) {
    if (!lapic_virt) return;
    *(volatile uint32_t*)((uintptr_t)lapic_virt + reg) = val;
}

uint32_t ioapic_read(uint32_t reg) {
    if (!ioapic_virt) return 0;
    volatile uint32_t *iowin = (volatile uint32_t*)((uintptr_t)ioapic_virt + 0x00U);
    volatile uint32_t *iodat = (volatile uint32_t*)((uintptr_t)ioapic_virt + 0x10U);
    *iowin = reg;
    return *iodat;
}

void ioapic_write(uint32_t reg, uint32_t val) {
    if (!ioapic_virt) return;
    volatile uint32_t *iowin = (volatile uint32_t*)((uintptr_t)ioapic_virt + 0x00U);
    volatile uint32_t *iodat = (volatile uint32_t*)((uintptr_t)ioapic_virt + 0x10U);
    *iowin = reg;
    *iodat = val;
}

void ioapic_route_irq(uint8_t irq, uint8_t vector, uint8_t dest) {
    if (!ioapic_virt) return;
    uint32_t reg_low = IOAPIC_REG_TABLE + (irq * 2U);
    uint32_t reg_high = IOAPIC_REG_TABLE + (irq * 2U) + 1U;

    ioapic_write(reg_high, (uint32_t)dest << 24);
    ioapic_write(reg_low, (uint32_t)vector);
}

void apic_disable_pic(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

int apic_is_enabled(void) {
    return apic_ready;
}

uint32_t apic_get_id(void) {
    if (!apic_ready) return 0;
    return apic_read(LAPIC_ID) >> 24;
}

void apic_eoi(void) {
    if (apic_ready) {
        apic_write(LAPIC_EOI, 0);
    }
}

void apic_timer_init(uint32_t frequency) {
    (void)frequency;
    if (!lapic_virt) return;
    /* Keep masked (bit 16) on vector 48 so PIT IRQ0 on vector 32 remains primary timer */
    apic_write(LAPIC_TIMER_DIVIDE, 0x03U);
    apic_write(LAPIC_TIMER, 48U | (1U << 16));
    apic_write(LAPIC_TIMER_INIT, 0);
}

static void apic_wait_icr_idle(void) {
    if (!lapic_virt) return;
    for (uint32_t i = 0; i < 100000U; i++) {
        if ((apic_read(LAPIC_ICR_LOW) & (1U << 12)) == 0) {
            return;
        }
        cpu_pause();
    }
}

void apic_send_init_ipi(uint8_t target_apic_id) {
    if (!apic_ready) return;
    apic_write(LAPIC_ESR, 0);
    apic_write(LAPIC_ICR_HIGH, (uint32_t)target_apic_id << 24);
    /* Delivery mode = INIT (0b101 << 8), level assert (1 << 14), edge trigger */
    apic_write(LAPIC_ICR_LOW, 0x00004500U);
    apic_wait_icr_idle();
}

void apic_send_sipi_ipi(uint8_t target_apic_id, uint8_t vector_page) {
    if (!apic_ready) return;
    apic_write(LAPIC_ESR, 0);
    apic_write(LAPIC_ICR_HIGH, (uint32_t)target_apic_id << 24);
    /* Delivery mode = StartUp (0b110 << 8), level assert (1 << 14), vector = physical_addr >> 12 */
    apic_write(LAPIC_ICR_LOW, 0x00004600U | (uint32_t)vector_page);
    apic_wait_icr_idle();
}

void apic_init(void) {
    if (!cpuid_has_feature(CPUID_FEAT_EDX_APIC)) {
        printk("APIC: Hardware does not report APIC support\n");
        apic_ready = 0;
        return;
    }

    uint64_t msr = rdmsr(MSR_IA32_APICBASE);
    if (!(msr & (1ULL << 11))) {
        msr |= (1ULL << 11);
        wrmsr(MSR_IA32_APICBASE, msr);
    }

    uint32_t base_addr = (uint32_t)(msr & 0xFFFFF000ULL);
    if (base_addr != 0) {
        lapic_phys = base_addr;
    }

    lapic_virt = (volatile uint32_t*)vmm_ioremap(lapic_phys, PAGE_SIZE);
    ioapic_virt = (volatile uint32_t*)vmm_ioremap(ioapic_phys, PAGE_SIZE);
    if (!lapic_virt || !ioapic_virt) {
        printk("APIC: Failed to ioremap LAPIC/IOAPIC MMIO\n");
        apic_ready = 0;
        return;
    }

    /* Enable Local APIC with spurious vector 0xFF while preserving 8259 PIC ExtINT routing */
    apic_write(LAPIC_TPR, 0);
    apic_write(LAPIC_SVR, 0x1FFU);
    apic_timer_init(1000);

    apic_ready = 1;

    uint32_t version = ioapic_read(IOAPIC_REG_VER);
    printk("APIC: Local APIC phys 0x%x (virt %p), ID %u\n",
           lapic_phys, (uint32_t)(uintptr_t)lapic_virt, apic_get_id());
    printk("APIC: I/O APIC phys 0x%x (virt %p), ver 0x%x, entries %u\n",
           ioapic_phys, (uint32_t)(uintptr_t)ioapic_virt,
           version & 0xFFU, ((version >> 16) & 0xFFU) + 1U);
}
