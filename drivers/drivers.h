/*
 * Dionnex Kernel Drivers Subsystem Framework Header
 */

#ifndef _DIONNEX_DRIVERS_H
#define _DIONNEX_DRIVERS_H

#include <ABI/kernel/internal/kabi.h>

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
	u32 device_id;
	u32 vendor_id;
	enum driver_type type;
	void *private_data;
	bool is_active;
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

/* Core Subsystem Initializers */
int drivers_subsystem_init(void);

/* Bus Drivers */
int pci_subsystem_init(void);
int pcie_subsystem_init(void);
int usb_subsystem_init(void);
int bus_peripherals_init(void);

/* CPU Support Drivers */
int cpu_intel_init(void);
int cpu_amd_init(void);
int cpu_arm_init(void);
int cpu_riscv_init(void);

/* GPU Drivers */
int gpu_amd_init(void);
int gpu_intel_init(void);
int gpu_nvidia_init(void);
int gpu_arm_init(void);
int gpu_display_init(void);

/* Storage Drivers */
int storage_block_init(void);
int storage_sata_init(void);
int storage_nvme_init(void);
int storage_scsi_init(void);
int storage_raid_init(void);
int storage_mobile_media_init(void);

/* Network Drivers */
int net_ethernet_init(void);
int net_wifi_init(void);
int net_bluetooth_init(void);
int net_cellular_init(void);
int net_accel_init(void);

/* Input Drivers */
int input_devices_init(void);

/* Audio Drivers */
int audio_subsystem_init(void);

/* Camera & ISP Drivers */
int camera_subsystem_init(void);

/* Mobile Drivers */
int mobile_subsystem_init(void);

/* Server Subsystem Drivers */
int server_subsystem_init(void);

/* Embedded Subsystem Drivers */
int embedded_subsystem_init(void);

/* Firmware Drivers */
int firmware_subsystem_init(void);

#endif /* _DIONNEX_DRIVERS_H */
