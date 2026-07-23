/* ============================================================================
 * Dionnex Monolithic Kernel - ATA/IDE PIO Hard Disk Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_ATA_H_
#define _DIONNEX_ATA_H_

#include <types.h>

#define ATA_PRIMARY_IO      0x1F0
#define ATA_PRIMARY_CTRL    0x3F6
#define ATA_SECONDARY_IO    0x170
#define ATA_SECONDARY_CTRL  0x376

struct ata_drive {
    uint8_t exists;
    uint8_t is_master;
    uint8_t bus; // 0=Primary, 1=Secondary
    uint32_t size_sectors;
    char model[41];
    char serial[21];
};

typedef struct ata_drive ata_drive_t;

void ata_init(void);
int ata_read_sectors(uint8_t bus, uint8_t drive, uint32_t lba, uint8_t count, uint8_t *buf);
int ata_write_sectors(uint8_t bus, uint8_t drive, uint32_t lba, uint8_t count, const uint8_t *buf);
ata_drive_t *ata_get_drive(uint8_t bus, uint8_t drive);
uint8_t ata_get_drive_count(void);
void ata_list_drives(void);

#endif /* _DIONNEX_ATA_H_ */
