/* ============================================================================
 * Dionnex Kernel - Driver Framework Device Manager (driver_framework/device.c)
 * ============================================================================
 */
#include <driver_framework/device.h>

static struct device_node device_table[MAX_DEVICES];
static int device_count = 0;

void driver_framework_init(void) {
    memset(device_table, 0, sizeof(device_table));
    device_count = 0;
    vga_puts("[DRIVER_FW] Framework de controladores de dispositivos inicializado.\n");
}

struct device_node* device_register(const char* name, uint32_t dev_id, uint32_t vendor_id) {
    if (device_count >= MAX_DEVICES) return NULL;

    struct device_node* dev = &device_table[device_count++];
    strncpy(dev->name, name, 31);
    dev->device_id = dev_id;
    dev->vendor_id = vendor_id;
    dev->is_initialized = 1;

    return dev;
}

int device_unregister(struct device_node* dev) {
    if (!dev) return -1;
    dev->is_initialized = 0;
    return 0;
}
