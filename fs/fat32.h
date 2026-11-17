/* ============================================================================
 * Dionnex Monolithic Kernel - FAT32 Filesystem Driver Header
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_FAT32_H_
#define _DIONNEX_FAT32_H_

#include <types.h>
#include <fs/vfs.h>

typedef int (*fat32_read_sector_fn)(uint64_t lba, uint16_t count, uint8_t *buf);

typedef struct fat32_bpb {
    uint8_t  jmp[3];
    char     oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} __attribute__((packed)) fat32_bpb_t;

typedef struct fat32_dir_entry {
    char     name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

int fat32_mount(fat32_read_sector_fn reader, uint32_t partition_lba);
int fat32_is_mounted(void);
void fat32_list_root(void);
int fat32_read_file(const char *filename, uint8_t *out_buf, uint32_t max_len, uint32_t *out_size);
vfs_node_t *fat32_get_root_node(void);

#endif /* _DIONNEX_FAT32_H_ */
