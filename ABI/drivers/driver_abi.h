/*
 * Dionnex Device Driver Framework ABI
 */

#ifndef _DIONNEX_DRIVER_ABI_H
#define _DIONNEX_DRIVER_ABI_H

#include <ABI/kernel/internal/kabi.h>

enum device_type {
	DEV_TYPE_CHAR,
	DEV_TYPE_BLOCK,
	DEV_TYPE_NET,
	DEV_TYPE_BUS
};

struct device;

struct file_operations {
	s64 (*read)(struct device *dev, char *buf, size_t count, u64 offset);
	s64 (*write)(struct device *dev, const char *buf, size_t count, u64 offset);
	int (*open)(struct device *dev);
	int (*release)(struct device *dev);
	s64 (*ioctl)(struct device *dev, u32 cmd, u64 arg);
};

struct device {
	const char *name;
	enum device_type type;
	dev_t dev_id;
	struct file_operations *fops;
	void *driver_data;
};

#endif /* _DIONOS_DRIVER_ABI_H */
