/* ============================================================================
 * Dionnex Monolithic Kernel - ATA/IDE PIO Hard Disk Driver Implementation
 * ============================================================================
 */
#include <drivers/ata.h>
#include <kernel/printk.h>
#include <string.h>
#include <io.h>

static ata_drive_t drives[4]; // Primary Master/Slave, Secondary Master/Slave
static uint8_t active_drive_count = 0;

static void ata_400ns_delay(uint16_t ctrl_port) {
    inb(ctrl_port);
    inb(ctrl_port);
    inb(ctrl_port);
    inb(ctrl_port);
}

static void ata_fix_string(char *str, int length) {
    for (int i = 0; i < length; i += 2) {
        char tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }
    str[length] = '\0';

    // Trim trailing spaces
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

    outb(io_base + 6, is_slave ? 0xB0 : 0xA0); // Select drive
    ata_400ns_delay(ctrl_base);

    outb(io_base + 2, 0);
    outb(io_base + 3, 0);
    outb(io_base + 4, 0);
    outb(io_base + 5, 0);

    outb(io_base + 7, 0xEC); // Send IDENTIFY command
    ata_400ns_delay(ctrl_base);

    uint8_t status = inb(io_base + 7);
    if (status == 0) return -1; // Drive does not exist

    while ((status & 0x80)) { // Wait for BSY to clear
        status = inb(io_base + 7);
    }

    uint8_t mid = inb(io_base + 4);
    uint8_t high = inb(io_base + 5);
    if (mid != 0 || high != 0) return -1; // Not ATA device (e.g. ATAPI)

    while (!(status & 0x08) && !(status & 0x01)) { // Wait for DRQ or ERR
        status = inb(io_base + 7);
    }

    if (status & 0x01) return -1; // Error

    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(io_base);
    }

    drive->exists = 1;
    drive->bus = bus;
    drive->is_master = !is_slave;

    // Serial: words 10-19
    memcpy(drive->serial, &identify_data[10], 20);
    ata_fix_string(drive->serial, 20);

    // Model: words 27-46
    memcpy(drive->model, &identify_data[27], 40);
    ata_fix_string(drive->model, 40);

    // Size: words 60-61 (28-bit LBA sector count)
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
                uint32_t megabytes = (drives[idx].size_sectors * 512) / (1024 * 1024);
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
    if (count == 0) return 0;

    uint16_t io_base = (bus == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (bus == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    outb(io_base + 6, 0xE0 | (drive_type << 4) | ((lba >> 24) & 0x0F));
    outb(io_base + 2, count);
    outb(io_base + 3, (uint8_t)lba);
    outb(io_base + 4, (uint8_t)(lba >> 8));
    outb(io_base + 5, (uint8_t)(lba >> 16));
    outb(io_base + 7, 0x20); // READ SECTORS

    uint16_t *ptr = (uint16_t*)buf;

    for (uint8_t i = 0; i < count; i++) {
        ata_400ns_delay(ctrl_base);
        uint8_t status = inb(io_base + 7);

        while ((status & 0x80) && !(status & 0x01)) { // Wait BSY clear
            status = inb(io_base + 7);
        }
        while (!(status & 0x08) && !(status & 0x01)) { // Wait DRQ set
            status = inb(io_base + 7);
        }

        if (status & 0x01) return -1; // ERR

        for (int j = 0; j < 256; j++) {
            *ptr++ = inw(io_base);
        }
    }
    return 0;
}

int ata_write_sectors(uint8_t bus, uint8_t drive_type, uint32_t lba, uint8_t count, const uint8_t *buf) {
    if (count == 0) return 0;

    uint16_t io_base = (bus == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (bus == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    outb(io_base + 6, 0xE0 | (drive_type << 4) | ((lba >> 24) & 0x0F));
    outb(io_base + 2, count);
    outb(io_base + 3, (uint8_t)lba);
    outb(io_base + 4, (uint8_t)(lba >> 8));
    outb(io_base + 5, (uint8_t)(lba >> 16));
    outb(io_base + 7, 0x30); // WRITE SECTORS

    const uint16_t *ptr = (const uint16_t*)buf;

    for (uint8_t i = 0; i < count; i++) {
        ata_400ns_delay(ctrl_base);
        uint8_t status = inb(io_base + 7);

        while ((status & 0x80) && !(status & 0x01)) {
            status = inb(io_base + 7);
        }
        while (!(status & 0x08) && !(status & 0x01)) {
            status = inb(io_base + 7);
        }

        if (status & 0x01) return -1;

        for (int j = 0; j < 256; j++) {
            outw(io_base, *ptr++);
        }
    }

    outb(io_base + 7, 0xE7); // Cache flush
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
            uint32_t mb = (drives[i].size_sectors * 512) / (1024 * 1024);
            printk("%-10s %-7s %-40s %-20s %u MB\n",
                   (drives[i].bus == 0) ? "Primary" : "Secondary",
                   drives[i].is_master ? "Master" : "Slave",
                   drives[i].model,
                   drives[i].serial,
                   mb);
        }
    }
}
