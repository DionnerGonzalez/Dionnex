/* ============================================================================
 * Dionnex Monolithic Kernel - ACPI Power Management Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_ACPI_H_
#define _DIONNEX_ACPI_H_

#include <types.h>

struct rsdp_descriptor {
    char signature[8]; // "RSD PTR "
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

void acpi_init(void);
void acpi_shutdown(void);
void acpi_reboot(void);
int acpi_is_available(void);

#endif /* _DIONNEX_ACPI_H_ */
