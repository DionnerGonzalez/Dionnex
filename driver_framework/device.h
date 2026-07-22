/* ============================================================================
 * Dionnex Kernel - Driver Framework Device Tree (driver_framework/device.h)
 * ============================================================================
 */
#ifndef _DIONNEX_DRIVER_FRAMEWORK_DEVICE_H
#define _DIONNEX_DRIVER_FRAMEWORK_DEVICE_H

#include <kernel/kernel.h>

#define MAX_DEVICES 64

struct device_node {
    char name[32];
    uint32_t device_id;
    uint32_t vendor_id;
    int is_initialized;
    void* private_data;
    struct device_node* parent;
};

void driver_framework_init(void);
struct device_node* device_register(const char* name, uint32_t dev_id, uint32_t vendor_id);
int device_unregister(struct device_node* dev);

#endif /* _DIONNEX_DRIVER_FRAMEWORK_DEVICE_H */
