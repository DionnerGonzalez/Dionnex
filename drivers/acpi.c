/* ============================================================================
 * Dionnex Monolithic Kernel - ACPI Power Management Driver Implementation
 * ============================================================================
 */
#include <drivers/acpi.h>
#include <kernel/printk.h>
#include <string.h>
#include <io.h>

static rsdp_descriptor_t *rsdp = NULL;
static fadt_descriptor_t *fadt = NULL;
static uint16_t SLP_TYPa = 0;
static uint16_t SLP_TYPb = 0;
static uint16_t SLP_EN   = 1 << 13; // Bit 13 is SLP_EN in PM1_CNT
static int acpi_enabled = 0;

static rsdp_descriptor_t *acpi_find_rsdp(uint32_t start, uint32_t length) {
    for (uint32_t addr = start; addr < start + length; addr += 16) {
        if (memcmp((void*)addr, "RSD PTR ", 8) == 0) {
            uint8_t *ptr = (uint8_t*)addr;
            uint8_t sum = 0;
            for (int i = 0; i < 20; i++) sum += ptr[i];
            if (sum == 0) return (rsdp_descriptor_t*)addr;
        }
    }
    return NULL;
}

void acpi_init(void) {
    rsdp = acpi_find_rsdp(0x9FC00, 1024);
    if (!rsdp) rsdp = acpi_find_rsdp(0xE0000, 0x20000);

    if (!rsdp) {
        printk("ACPI: RSDP structure not found\n");
        acpi_enabled = 0;
        return;
    }

    acpi_header_t *rsdt = (acpi_header_t*)rsdp->rsdt_address;
    if (memcmp(rsdt->signature, "RSDT", 4) != 0) {
        printk("ACPI: Invalid RSDT signature\n");
        acpi_enabled = 0;
        return;
    }

    uint32_t entries = (rsdt->length - sizeof(acpi_header_t)) / 4;
    uint32_t *table_ptrs = (uint32_t*)((uintptr_t)rsdt + sizeof(acpi_header_t));

    for (uint32_t i = 0; i < entries; i++) {
        acpi_header_t *header = (acpi_header_t*)table_ptrs[i];
        if (memcmp(header->signature, "FACP", 4) == 0) {
            fadt = (fadt_descriptor_t*)header;
            break;
        }
    }

    if (fadt) {
        acpi_enabled = 1;
        SLP_TYPa = (0x05 << 10); // Default QEMU S5 sleep type
        SLP_TYPb = (0x05 << 10);
        printk("ACPI: Revision %u, FADT at 0x%x\n", rsdp->revision, (uint32_t)fadt);
    } else {
        printk("ACPI: FADT table not found\n");
        acpi_enabled = 0;
    }
}

int acpi_is_available(void) {
    return acpi_enabled;
}

void acpi_shutdown(void) {
    printk("ACPI: Initiating system shutdown...\n");

    if (fadt && fadt->PM1a_CNT_BLK) {
        outw((uint16_t)fadt->PM1a_CNT_BLK, SLP_TYPa | SLP_EN);
        if (fadt->PM1b_CNT_BLK) {
            outw((uint16_t)fadt->PM1b_CNT_BLK, SLP_TYPb | SLP_EN);
        }
    }

    // QEMU ACPI Shutdown Fallbacks
    outw(0x604, 0x2000);  // QEMU modern
    outw(0xB004, 0x2000); // QEMU legacy
    outw(0x4004, 0x3400); // VirtualBox

    // Halt loop if shutdown port fails
    asm volatile ("cli; hlt");
}

void acpi_reboot(void) {
    printk("ACPI: Initiating system reboot...\n");

    // Keyboard Controller Reset
    uint8_t good = 0x02;
    while (good & 0x02) good = inb(0x64);
    outb(0x64, 0xFE);

    // Fallback: Triple Fault
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) null_idt = {0, 0};

    asm volatile ("lidt %0; int $3" :: "m"(null_idt));

    // Halt loop
    asm volatile ("cli; hlt");
}
