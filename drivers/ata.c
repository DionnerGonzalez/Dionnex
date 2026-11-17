/* ============================================================================
 * Dionnex Monolithic Kernel - ATA/IDE PIO Hard Disk Driver Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/ata.h>
#include <kernel/printk.h>
#include <string.h>
#include <io.h>

#define ATA_POLL_TIMEOUT 1000000U

static ata_drive_t drives[4];
static uint8_t active_drive_count = 0;

static void ata_400ns_delay(uint16_t ctrl_port) {
    inb(ctrl_port);
    inb(ctrl_port);
    inb(ctrl_port);
    inb(ctrl_port);
}

static int ata_wait_not_bsy(uint16_t io_base) {
    for (uint32_t i = 0; i < ATA_POLL_TIMEOUT; i++) {
        uint8_t status = inb(io_base + 7);
        if (!(status & 0x80U)) return 0;
        if (status & 0x01U) return -1;
    }
    return -1;
}

static int ata_wait_drq(uint16_t io_base) {
    for (uint32_t i = 0; i < ATA_POLL_TIMEOUT; i++) {
        uint8_t status = inb(io_base + 7);
        if (status & 0x01U) return -1;
        if (status & 0x20U) return -1; /* Device Fault */
        if (!(status & 0x80U) && (status & 0x08U)) return 0;
    }
    return -1;
}

static void ata_fix_string(char *str, int length) {
    for (int i = 0; i < length; i += 2) {
        char tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }
    str[length] = '\0';

    for (int i = length - 1; i >= 0; i--) {
        if (str[i] == ' ' || str[i] == '\0') {
            str[i] = '\0';
        } else {
            break;
        }
    }
}

static int ata_identify(uint8_t bus, uint8_t is_slave, ata_drive_t *drive) {
    uint16_t io_base = (bus == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (bus == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    outb(io_base + 6, is_slave ? 0xB0 : 0xA0);
    ata_400ns_delay(ctrl_base);

    outb(io_base + 2, 0);
    outb(io_base + 3, 0);
    outb(io_base + 4, 0);
    outb(io_base + 5, 0);

    outb(io_base + 7, 0xEC);
    ata_400ns_delay(ctrl_base);

    uint8_t status = inb(io_base + 7);
    if (status == 0 || status == 0xFF) return -1;

    if (ata_wait_not_bsy(io_base) != 0) return -1;

    uint8_t mid = inb(io_base + 4);
    uint8_t high = inb(io_base + 5);
    if (mid != 0 || high != 0) return -1;

    if (ata_wait_drq(io_base) != 0) return -1;

    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(io_base);
    }

    drive->exists = 1;
    drive->bus = bus;
    drive->is_master = !is_slave;

    memcpy(drive->serial, &identify_data[10], 20);
    ata_fix_string(drive->serial, 20);

    memcpy(drive->model, &identify_data[27], 40);
    ata_fix_string(drive->model, 40);

    drive->size_sectors = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    return 0;
}

void ata_init(void) {
    active_drive_count = 0;
    memset(drives, 0, sizeof(drives));

    int idx = 0;
    for (uint8_t bus = 0; bus < 2; bus++) {
        for (uint8_t slave = 0; slave < 2; slave++) {
            if (ata_identify(bus, slave, &drives[idx]) == 0) {
                uint32_t megabytes = drives[idx].size_sectors / 2048U;
                printk("ATA: %s %s - %s (%u MB)\n",
                       (bus == 0) ? "Primary" : "Secondary",
                       slave ? "Slave" : "Master",
                       drives[idx].model,
                       megabytes);
                active_drive_count++;
            }
            idx++;
        }
    }

    if (active_drive_count == 0) {
        printk("ATA: No drives detected\n");
    }
}

int ata_read_sectors(uint8_t bus, uint8_t drive_type, uint32_t lba, uint8_t count, uint8_t *buf) {
    if (count == 0 || !buf || bus >= 2 || drive_type >= 2) return -1;
    if (!drives[(bus * 2) + drive_type].exists) return -1;

    uint16_t io_base = (bus == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (bus == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    if (ata_wait_not_bsy(io_base) != 0) return -1;

    outb(io_base + 6, 0xE0 | (drive_type << 4) | ((lba >> 24) & 0x0F));
    ata_400ns_delay(ctrl_base);
    outb(io_base + 2, count);
    outb(io_base + 3, (uint8_t)lba);
    outb(io_base + 4, (uint8_t)(lba >> 8));
    outb(io_base + 5, (uint8_t)(lba >> 16));
    outb(io_base + 7, 0x20);

    uint16_t *ptr = (uint16_t*)buf;

    for (uint8_t i = 0; i < count; i++) {
        ata_400ns_delay(ctrl_base);
        if (ata_wait_drq(io_base) != 0) return -1;

        for (int j = 0; j < 256; j++) {
            *ptr++ = inw(io_base);
        }
    }
    return 0;
}

int ata_write_sectors(uint8_t bus, uint8_t drive_type, uint32_t lba, uint8_t count, const uint8_t *buf) {
    if (count == 0 || !buf || bus >= 2 || drive_type >= 2) return -1;
    if (!drives[(bus * 2) + drive_type].exists) return -1;

    uint16_t io_base = (bus == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (bus == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    if (ata_wait_not_bsy(io_base) != 0) return -1;

    outb(io_base + 6, 0xE0 | (drive_type << 4) | ((lba >> 24) & 0x0F));
    ata_400ns_delay(ctrl_base);
    outb(io_base + 2, count);
    outb(io_base + 3, (uint8_t)lba);
    outb(io_base + 4, (uint8_t)(lba >> 8));
    outb(io_base + 5, (uint8_t)(lba >> 16));
    outb(io_base + 7, 0x30);

    const uint16_t *ptr = (const uint16_t*)buf;

    for (uint8_t i = 0; i < count; i++) {
        ata_400ns_delay(ctrl_base);
        if (ata_wait_drq(io_base) != 0) return -1;

        for (int j = 0; j < 256; j++) {
            outw(io_base, *ptr++);
        }
    }

    outb(io_base + 7, 0xE7);
    ata_wait_not_bsy(io_base);
    return 0;
}

ata_drive_t *ata_get_drive(uint8_t bus, uint8_t drive_idx) {
    uint8_t idx = (bus * 2) + drive_idx;
    if (idx < 4 && drives[idx].exists) {
        return &drives[idx];
    }
    return NULL;
}

uint8_t ata_get_drive_count(void) {
    return active_drive_count;
}

void ata_list_drives(void) {
    printk("BUS        ROLE    MODEL                                    SERIAL                SIZE\n");
    printk("---        ----    -----                                    ------                ----\n");

    for (int i = 0; i < 4; i++) {
        if (drives[i].exists) {
            uint32_t mb = drives[i].size_sectors / 2048U;
            printk("%-10s %-7s %-40s %-20s %u MB\n",
                   (drives[i].bus == 0) ? "Primary" : "Secondary",
                   drives[i].is_master ? "Master" : "Slave",
                   drives[i].model,
                   drives[i].serial,
                   mb);
        }
    }
}
