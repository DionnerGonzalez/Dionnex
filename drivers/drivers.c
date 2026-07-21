/*
 * Dionnex Kernel Drivers Subsystem Manager
 */

#include <drivers/drivers.h>

extern void pr_info(const char *fmt, ...);

int drivers_subsystem_init(void)
{
	pr_info("[DRIVERS] Initializing Dionnex Unified Driver Architecture Framework...\n");

	/* 1. Bus Subsystems */
	pci_subsystem_init();
	pcie_subsystem_init();
	usb_subsystem_init();
	bus_peripherals_init();

	/* 2. CPU Architectures & PMUs */
	cpu_intel_init();
	cpu_amd_init();
	cpu_arm_init();
	cpu_riscv_init();

	/* 3. Graphics Processing Units & Display Engines */
	gpu_amd_init();
	gpu_intel_init();
	gpu_nvidia_init();
	gpu_arm_init();
	gpu_display_init();

	/* 4. Storage Controllers & NVMe/SATA/RAID */
	storage_block_init();
	storage_sata_init();
	storage_nvme_init();
	storage_scsi_init();
	storage_raid_init();
	storage_mobile_media_init();

	/* 5. Network Stack Drivers */
	net_ethernet_init();
	net_wifi_init();
	net_bluetooth_init();
	net_cellular_init();
	net_accel_init();

	/* 6. Human Interface Devices */
	input_devices_init();

	/* 7. Sound & Audio Core */
	audio_subsystem_init();

	/* 8. Camera & ISP Pipeline */
	camera_subsystem_init();

	/* 9. Mobile Platform Subsystems */
	mobile_subsystem_init();

	/* 10. Enterprise Server Controllers */
	server_subsystem_init();

	/* 11. Embedded & Industrial Control */
	embedded_subsystem_init();

	/* 12. Hardware Firmware & Microcode Engine */
	firmware_subsystem_init();

	pr_info("[DRIVERS] All modular hardware drivers registered successfully.\n");
	return 0;
}
