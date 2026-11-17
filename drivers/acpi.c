/* ============================================================================
 * Dionnex Monolithic Kernel - ACPI Power Management & Table Parser
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/acpi.h>
#include <kernel/printk.h>
#include <dionnex_rust.h>
#include <mm/vmm.h>
#include <string.h>
#include <io.h>

#define ACPI_MAX_CACHED_TABLES 16

static rsdp_descriptor_t *rsdp = NULL;
static acpi_header_t *rsdt_virt = NULL;
static fadt_descriptor_t *fadt = NULL;
static acpi_header_t *cached_tables[ACPI_MAX_CACHED_TABLES];
static uint32_t cached_table_count = 0;

static uint16_t SLP_TYPa = 0;
static uint16_t SLP_TYPb = 0;
static uint16_t SLP_EN   = 1 << 13;
static int acpi_enabled = 0;

static int acpi_checksum_valid(const uint8_t *ptr, uint32_t length) {
    return dionnex_rust_acpi_checksum_ok(ptr, length);
}

static rsdp_descriptor_t *acpi_find_rsdp(uint32_t start, uint32_t length) {
    for (uint32_t addr = start; addr < start + length; addr += 16) {
        if (memcmp((void*)(uintptr_t)addr, "RSD PTR ", 8) == 0) {
            if (acpi_checksum_valid((const uint8_t*)(uintptr_t)addr, 20)) {
                return (rsdp_descriptor_t*)(uintptr_t)addr;
            }
        }
    }
    return NULL;
}

static acpi_header_t *acpi_map_table(uint32_t phys_addr) {
    if (phys_addr == 0) return NULL;
    acpi_header_t *hdr = (acpi_header_t*)vmm_ioremap(phys_addr, sizeof(acpi_header_t));
    if (!hdr) return NULL;
    uint32_t len = hdr->length;
    if (len < sizeof(acpi_header_t) || len > (256U * 1024U)) return NULL;
    acpi_header_t *full = (acpi_header_t*)vmm_ioremap(phys_addr, len);
    if (!full) return NULL;
    if (!acpi_checksum_valid((const uint8_t*)full, len)) {
        return NULL;
    }
    return full;
}

void acpi_init(void) {
    cached_table_count = 0;
    fadt = NULL;
    rsdt_virt = NULL;

    rsdp = acpi_find_rsdp(0x9FC00U, 1024U);
    if (!rsdp) rsdp = acpi_find_rsdp(0xE0000U, 0x20000U);

    if (!rsdp) {
        printk("ACPI: RSDP structure not found\n");
        acpi_enabled = 0;
        return;
    }

    rsdt_virt = acpi_map_table(rsdp->rsdt_address);
    if (!rsdt_virt || memcmp(rsdt_virt->signature, "RSDT", 4) != 0) {
        printk("ACPI: Invalid or unmappable RSDT at 0x%x\n", rsdp->rsdt_address);
        acpi_enabled = 0;
        return;
    }

    uint32_t entries = (rsdt_virt->length - sizeof(acpi_header_t)) / 4U;
    uint32_t *table_ptrs = (uint32_t*)((uintptr_t)rsdt_virt + sizeof(acpi_header_t));

    for (uint32_t i = 0; i < entries; i++) {
        acpi_header_t *hdr = acpi_map_table(table_ptrs[i]);
        if (!hdr) continue;

        if (cached_table_count < ACPI_MAX_CACHED_TABLES) {
            cached_tables[cached_table_count++] = hdr;
        }
        if (memcmp(hdr->signature, "FACP", 4) == 0) {
            fadt = (fadt_descriptor_t*)hdr;
        }
    }

    if (fadt) {
        acpi_enabled = 1;
        SLP_TYPa = (0x05 << 10);
        SLP_TYPb = (0x05 << 10);
        printk("ACPI: Revision %u, %u tables mapped, FADT at 0x%x\n",
               rsdp->revision, cached_table_count, (uint32_t)(uintptr_t)fadt);
    } else {
        printk("ACPI: FADT table not found (%u tables mapped)\n", cached_table_count);
        acpi_enabled = 0;
    }
}

acpi_header_t *acpi_find_table(const char *signature) {
    if (!signature) return NULL;
    for (uint32_t i = 0; i < cached_table_count; i++) {
        if (cached_tables[i] && memcmp(cached_tables[i]->signature, signature, 4) == 0) {
            return cached_tables[i];
        }
    }
    return NULL;
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

    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);

    asm volatile ("cli; hlt");
}

void acpi_reboot(void) {
    printk("ACPI: Initiating system reboot...\n");

    uint8_t good = 0x02;
    for (uint32_t i = 0; i < 100000U && (good & 0x02); i++) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);

    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) null_idt = {0, 0};

    asm volatile ("lidt %0; int $3" :: "m"(null_idt));
    asm volatile ("cli; hlt");
}
