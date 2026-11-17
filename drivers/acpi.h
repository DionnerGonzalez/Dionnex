/* ============================================================================
 * Dionnex Monolithic Kernel - ACPI Power Management & Table Parser Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_ACPI_H_
#define _DIONNEX_ACPI_H_

#include <types.h>

struct rsdp_descriptor {
    char signature[8]; /* "RSD PTR " */
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

typedef struct rsdp_descriptor rsdp_descriptor_t;

struct acpi_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

typedef struct acpi_header acpi_header_t;

struct fadt_descriptor {
    acpi_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;
    uint8_t  preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_CNT;
    uint32_t PM1a_EVT_BLK;
    uint32_t PM1b_EVT_BLK;
    uint32_t PM1a_CNT_BLK;
    uint32_t PM1b_CNT_BLK;
    uint32_t PM2_CNT_BLK;
    uint32_t PM_TMR_BLK;
    uint32_t GPE0_BLK;
    uint32_t GPE1_BLK;
    uint8_t  PM1_EVT_LEN;
    uint8_t  PM1_CNT_LEN;
} __attribute__((packed));

typedef struct fadt_descriptor fadt_descriptor_t;

typedef struct acpi_gas {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed)) acpi_gas_t;

typedef struct acpi_hpet_table {
    acpi_header_t header;
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size     : 1;
    uint8_t reserved         : 1;
    uint8_t legacy_replace   : 1;
    uint16_t pci_vendor_id;
    acpi_gas_t address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed)) acpi_hpet_table_t;

typedef struct acpi_mcfg_alloc {
    uint64_t base_address;
    uint16_t pci_segment_group;
    uint8_t  start_bus_number;
    uint8_t  end_bus_number;
    uint32_t reserved;
} __attribute__((packed)) acpi_mcfg_alloc_t;

typedef struct acpi_mcfg_table {
    acpi_header_t header;
    uint64_t reserved;
} __attribute__((packed)) acpi_mcfg_table_t;

typedef struct acpi_madt_header {
    acpi_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_header_t;

void acpi_init(void);
void acpi_shutdown(void);
void acpi_reboot(void);
int acpi_is_available(void);
acpi_header_t *acpi_find_table(const char *signature);

#endif /* _DIONNEX_ACPI_H_ */
