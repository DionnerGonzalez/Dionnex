/*
 * Dionnex Kernel Drivers Subsystem Framework Header
 */

#ifndef _DIONNEX_DRIVERS_H
#define _DIONNEX_DRIVERS_H

#include <kernel.h>

#define DRIVER_NAME_MAX 64
#define DEVICE_NAME_MAX 64

enum driver_type {
	DRIVER_TYPE_BUS = 0,
	DRIVER_TYPE_CPU,
	DRIVER_TYPE_GPU,
	DRIVER_TYPE_STORAGE,
	DRIVER_TYPE_NETWORK,
	DRIVER_TYPE_INPUT,
	DRIVER_TYPE_AUDIO,
	DRIVER_TYPE_CAMERA,
	DRIVER_TYPE_MOBILE,
	DRIVER_TYPE_SERVER,
	DRIVER_TYPE_EMBEDDED,
	DRIVER_TYPE_FIRMWARE,
};

enum driver_state {
	DRIVER_STATE_UNINITIALIZED = 0,
	DRIVER_STATE_REGISTERED,
	DRIVER_STATE_ACTIVE,
	DRIVER_STATE_SUSPENDED,
	DRIVER_STATE_ERROR,
};

struct driver_device {
	char name[DEVICE_NAME_MAX];
	uint32_t device_id;
	uint32_t vendor_id;
	enum driver_type type;
	void *private_data;
	int is_active;
};

struct driver_module {
	char name[DRIVER_NAME_MAX];
	enum driver_type type;
	enum driver_state state;
	int (*init)(void);
	int (*probe)(struct driver_device *dev);
	int (*remove)(struct driver_device *dev);
	int (*shutdown)(struct driver_device *dev);
};

int drivers_subsystem_init(void);

#endif /* _DIONNEX_DRIVERS_H */

