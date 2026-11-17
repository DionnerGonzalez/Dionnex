/* ============================================================================
 * Dionnex Monolithic Kernel - FAT32 Filesystem Driver Implementation
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <fs/fat32.h>
#include <kernel/printk.h>
#include <mm/heap.h>
#include <string.h>

static fat32_read_sector_fn g_reader = NULL;
static uint32_t g_part_lba = 0;
static fat32_bpb_t g_bpb;
static uint32_t g_fat_start_lba = 0;
static uint32_t g_data_start_lba = 0;
static uint32_t g_mounted = 0;
static vfs_node_t g_fat32_root;

static uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    return g_data_start_lba + ((cluster - 2U) * g_bpb.sectors_per_cluster);
}

static uint32_t fat32_next_cluster(uint32_t cluster) {
    uint8_t sec[512];
    uint32_t fat_offset = cluster * 4U;
    uint32_t fat_sector = g_fat_start_lba + (fat_offset / 512U);
    uint32_t ent_offset = fat_offset % 512U;

    if (!g_reader || g_reader(fat_sector, 1U, sec) != 0) {
        return 0x0FFFFFFFU;
    }
    uint32_t val = *(const uint32_t*)(sec + ent_offset);
    return val & 0x0FFFFFFFU;
}

static void fat32_format_83(const char *raw11, char *out) {
    int p = 0;
    for (int i = 0; i < 8 && raw11[i] != ' '; i++) {
        out[p++] = raw11[i];
    }
    if (raw11[8] != ' ') {
        out[p++] = '.';
        for (int i = 8; i < 11 && raw11[i] != ' '; i++) {
            out[p++] = raw11[i];
        }
    }
    out[p] = '\0';
}

static int fat32_ci_equal(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 'a' + 'A');
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == *b;
}

int fat32_mount(fat32_read_sector_fn reader, uint32_t partition_lba) {
    if (!reader) return -1;
    uint8_t sec0[512];
    if (reader(partition_lba, 1U, sec0) != 0) return -1;

    if (sec0[510] != 0x55U || sec0[511] != 0xAAU) {
        return -1;
    }

    memcpy(&g_bpb, sec0, sizeof(fat32_bpb_t));
    if (g_bpb.bytes_per_sector != 512U || g_bpb.sectors_per_cluster == 0 ||
        g_bpb.fat_size_32 == 0 || g_bpb.root_cluster < 2U) {
        return -1;
    }

    g_reader = reader;
    g_part_lba = partition_lba;
    g_fat_start_lba = partition_lba + g_bpb.reserved_sectors;
    g_data_start_lba = g_fat_start_lba + ((uint32_t)g_bpb.num_fats * g_bpb.fat_size_32);
    g_mounted = 1;

    memset(&g_fat32_root, 0, sizeof(g_fat32_root));
    strcpy(g_fat32_root.name, "fat32");
    g_fat32_root.flags = VFS_DIR;

    printk("FAT32: Mounted volume at LBA %u (cluster=%u sec, root_clus=%u)\n",
           partition_lba, g_bpb.sectors_per_cluster, g_bpb.root_cluster);
    return 0;
}

int fat32_is_mounted(void) { return (int)g_mounted; }
vfs_node_t *fat32_get_root_node(void) { return g_mounted ? &g_fat32_root : NULL; }

void fat32_list_root(void) {
    if (!g_mounted) {
        printk("FAT32: No volume mounted\n");
        return;
    }

    uint8_t sec[512];
    uint32_t cluster = g_bpb.root_cluster;
    while (cluster >= 2U && cluster < 0x0FFFFFF8U) {
        uint32_t lba = fat32_cluster_to_lba(cluster);
        for (uint8_t s = 0; s < g_bpb.sectors_per_cluster; s++) {
            if (g_reader(lba + s, 1U, sec) != 0) return;
            const fat32_dir_entry_t *ents = (const fat32_dir_entry_t*)sec;
            for (int i = 0; i < 16; i++) {
                uint8_t first = (uint8_t)ents[i].name[0];
                if (first == 0x00U) return;
                if (first == 0xE5U || (ents[i].attr & 0x0FU) == 0x0FU || (ents[i].attr & 0x08U)) {
                    continue;
                }
                char name[16];
                fat32_format_83(ents[i].name, name);
                printk("  %-14s %s %u bytes\n", name,
                       (ents[i].attr & 0x10U) ? "<DIR>" : "     ",
                       ents[i].file_size);
            }
        }
        cluster = fat32_next_cluster(cluster);
    }
}

int fat32_read_file(const char *filename, uint8_t *out_buf, uint32_t max_len, uint32_t *out_size) {
    if (!g_mounted || !filename || !out_buf) return -1;

    uint8_t sec[512];
    uint32_t cluster = g_bpb.root_cluster;
    uint32_t start_clus = 0;
    uint32_t fsize = 0;
    int found = 0;

    while (cluster >= 2U && cluster < 0x0FFFFFF8U && !found) {
        uint32_t lba = fat32_cluster_to_lba(cluster);
        for (uint8_t s = 0; s < g_bpb.sectors_per_cluster && !found; s++) {
            if (g_reader(lba + s, 1U, sec) != 0) return -1;
            const fat32_dir_entry_t *ents = (const fat32_dir_entry_t*)sec;
            for (int i = 0; i < 16; i++) {
                uint8_t first = (uint8_t)ents[i].name[0];
                if (first == 0x00U) { cluster = 0x0FFFFFFFU; break; }
                if (first == 0xE5U || (ents[i].attr & 0x0FU) == 0x0FU || (ents[i].attr & 0x18U)) {
                    continue;
                }
                char name[16];
                fat32_format_83(ents[i].name, name);
                if (fat32_ci_equal(name, filename)) {
                    start_clus = ((uint32_t)ents[i].fst_clus_hi << 16) | ents[i].fst_clus_lo;
                    fsize = ents[i].file_size;
                    found = 1;
                    break;
                }
            }
        }
        if (!found) cluster = fat32_next_cluster(cluster);
    }

    if (!found) return -1;
    if (out_size) *out_size = fsize;

    uint32_t to_read = (fsize < max_len) ? fsize : max_len;
    uint32_t copied = 0;
    cluster = start_clus;

    while (copied < to_read && cluster >= 2U && cluster < 0x0FFFFFF8U) {
        uint32_t lba = fat32_cluster_to_lba(cluster);
        for (uint8_t s = 0; s < g_bpb.sectors_per_cluster && copied < to_read; s++) {
            if (g_reader(lba + s, 1U, sec) != 0) return -1;
            uint32_t chunk = to_read - copied;
            if (chunk > 512U) chunk = 512U;
            memcpy(out_buf + copied, sec, chunk);
            copied += chunk;
        }
        cluster = fat32_next_cluster(cluster);
    }
    return (int)copied;
}
