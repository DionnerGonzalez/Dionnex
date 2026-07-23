/* ============================================================================
 * Dionnex Monolithic Kernel - RAM File System (RAMFS) Implementation
 * ============================================================================
 */
#include <fs/ramfs.h>
#include <kernel/printk.h>

static ramfs_file_t ramfs_files[RAMFS_MAX_FILES];

static int ramfs_strcmp(const char *a, const char *b) {
    if (!a || !b) return -1;
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

static void ramfs_strcpy(char *dst, const char *src) {
    if (!dst || !src) return;
    uint32_t i = 0;
    while (src[i] && i < RAMFS_MAX_NAME - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void ramfs_init(void) {
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        ramfs_files[i].used = 0;
        ramfs_files[i].size = 0;
        ramfs_files[i].name[0] = '\0';
    }

    // Populate standard welcome file
    ramfs_create("readme.txt");
    const char *welcome = "Welcome to Dionnex Monolithic Kernel v0.1!";
    ramfs_write("readme.txt", (const uint8_t*)welcome, 42);

    printk("RAMFS: Initialized in-memory Virtual Filesystem (%d slots)\n", RAMFS_MAX_FILES);
}

int ramfs_find(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used && ramfs_strcmp(ramfs_files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int ramfs_create(const char *name) {
    if (!name) return -1;
    if (ramfs_find(name) >= 0) return -1; // File already exists

    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (!ramfs_files[i].used) {
            ramfs_files[i].used = 1;
            ramfs_files[i].size = 0;
            ramfs_strcpy(ramfs_files[i].name, name);
            ramfs_files[i].data[0] = '\0';
            return 0;
        }
    }
    return -1; // No free file slots
}

int ramfs_delete(const char *name) {
    int idx = ramfs_find(name);
    if (idx < 0) return -1;

    ramfs_files[idx].used = 0;
    ramfs_files[idx].size = 0;
    ramfs_files[idx].name[0] = '\0';
    return 0;
}

int ramfs_read(const char *name, uint8_t *buf, uint32_t len) {
    int idx = ramfs_find(name);
    if (idx < 0 || !buf) return -1;

    uint32_t to_read = (len < ramfs_files[idx].size) ? len : ramfs_files[idx].size;
    for (uint32_t i = 0; i < to_read; i++) {
        buf[i] = ramfs_files[idx].data[i];
    }
    return (int)to_read;
}

int ramfs_write(const char *name, const uint8_t *buf, uint32_t len) {
    int idx = ramfs_find(name);
    if (idx < 0 || !buf) return -1;

    uint32_t to_write = (len > RAMFS_MAX_DATA) ? RAMFS_MAX_DATA : len;
    for (uint32_t i = 0; i < to_write; i++) {
        ramfs_files[idx].data[i] = buf[i];
    }
    ramfs_files[idx].size = to_write;
    return (int)to_write;
}

void ramfs_list(void) {
    printk("NAME                 SIZE\n");
    printk("----                 ----\n");

    int count = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used) {
            printk("%s          %u bytes\n", ramfs_files[i].name, ramfs_files[i].size);
            count++;
        }
    }
    if (count == 0) {
        printk("(empty)\n");
    }
}

uint32_t ramfs_get_file_count(void) {
    uint32_t count = 0;
    for (int i = 0; i < RAMFS_MAX_FILES; i++) {
        if (ramfs_files[i].used) count++;
    }
    return count;
}
