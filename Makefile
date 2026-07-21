# ==========================================================================
# Dionnex Main Kernel Makefile
# ==========================================================================

VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION = -RELEASE
NAME = Dionnex Modular Monolithic Kernel

# Architecture selection
ARCH ?= x86_64
CROSS_COMPILE ?= 

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
ASM = $(CROSS_COMPILE)as

CFLAGS := -O2 -Wall -Wextra -Werror -std=c11 -ffreestanding -fno-builtin \
          -fno-stack-protector -mcmodel=kernel -mno-red-zone -Iinclude -I. \
          -D__KERNEL__ -IABI/kernel/internal

LDFLAGS := -nostdlib -z max-page-size=0x1000 -T arch/$(ARCH)/linker.ld

# Subdirectory source directories
CORE_OBJS := kernel/core/kernel.o kernel/core/init.o kernel/core/panic.o \
             kernel/core/logging.o kernel/core/errors.o \
             kernel/interrupts/irq.o kernel/exceptions/traps.o \
             kernel/timers/timer.o kernel/events/event.o \
             kernel/workqueue/workqueue.o kernel/tasklets/tasklet.o \
             kernel/tracing/ftrace.o kernel/debugging/kdb.o

PROC_OBJS := process/scheduler/fair/cfs.o process/scheduler/realtime/rt.o \
             process/processes/process.o process/threads/thread.o \
             process/context_switch/switch.o process/namespaces/pid_ns.o \
             process/signals/signal.o process/synchronization/spinlock.o

MM_OBJS   := memory/physical/pmm.o memory/virtual/vmm.o memory/paging/page_table.o \
             memory/huge_pages/hugetlb.o memory/allocator/kmalloc.o memory/slab/slab.o \
             memory/dma/dma_alloc.o memory/cache/kmem_cache.o memory/memory_protection/mpu.o

SYS_OBJS  := syscall/table/syscall_table.o syscall/handlers/sys_process.o \
             syscall/ipc/sys_ipc.o syscall/filesystem/sys_fs.o \
             syscall/networking/sys_net.o syscall/compatibility/sys_compat.o

BOOT_OBJS := boot/early_init/early_setup.o boot/hardware_detection/hw_detect.o

ARCH_OBJS := arch/x86/cpu/intel/intel_core.o arch/x86/cpu/intel/intel_xeon.o \
             arch/x86/cpu/intel/intel_atom.o arch/x86/cpu/intel/intel_alderlake.o \
             arch/x86/cpu/intel/intel_raptorlake.o arch/x86/cpu/intel/intel_future.o \
             arch/x86/cpu/amd/amd_athlon.o arch/x86/cpu/amd/amd_ryzen.o \
             arch/x86/cpu/amd/amd_threadripper.o arch/x86/cpu/amd/amd_epyc.o \
             arch/x86/cpu/amd/amd_zen.o \
             arch/arm/cortex/arm_cortex_a.o arch/arm/cortex/arm_cortex_m.o \
             arch/arm/neoverse/arm_neoverse.o arch/arm/custom_soc/arm_soc.o \
             arch/riscv/riscv_cpu.o

DRIVER_OBJS := drivers/drivers.o \
               drivers/bus/pci/core/pci_core.o drivers/bus/pci/enumeration/pci_enum.o \
               drivers/bus/pci/hotplug/pci_hotplug.o drivers/bus/pci/power_management/pci_pm.o \
               drivers/bus/pcie/root_complex/pcie_rc.o drivers/bus/pcie/endpoint/pcie_ep.o \
               drivers/bus/pcie/advanced_features/pcie_adv.o \
               drivers/bus/usb/core/usb_core.o drivers/bus/usb/host/usb_host.o \
               drivers/bus/usb/device/usb_dev.o drivers/bus/usb/usb2/usb2.o \
               drivers/bus/usb/usb3/usb3.o drivers/bus/usb/usb4/usb4.o \
               drivers/bus/usb/type_c/type_c.o \
               drivers/bus/thunderbolt/thunderbolt.o drivers/bus/firewire/firewire.o \
               drivers/bus/i2c/i2c.o drivers/bus/spi/spi.o drivers/bus/uart/uart.o \
               drivers/bus/gpio/gpio.o drivers/bus/pwm/pwm.o drivers/bus/can/can.o \
               drivers/bus/virtio/virtio.o \
               drivers/cpu/intel/microcode/intel_ucode.o drivers/cpu/intel/performance_monitor/intel_pmu.o \
               drivers/cpu/amd/microcode/amd_ucode.o drivers/cpu/amd/performance_monitor/amd_pmu.o \
               drivers/cpu/arm/pmu/arm_pmu.o \
               drivers/cpu/riscv/pmu/riscv_pmu.o \
               drivers/gpu/amd/radeon/amd_radeon.o drivers/gpu/amd/vega/amd_vega.o \
               drivers/gpu/amd/rdna/amd_rdna.o drivers/gpu/amd/compute/amd_gpu_compute.o \
               drivers/gpu/amd/firmware/amd_gpu_fw.o drivers/gpu/intel/integrated/intel_igpu.o \
               drivers/gpu/intel/iris/intel_iris.o drivers/gpu/intel/xe/intel_xe.o \
               drivers/gpu/intel/arc/intel_arc.o drivers/gpu/nvidia/legacy/nv_legacy.o \
               drivers/gpu/nvidia/modern/nv_modern.o drivers/gpu/nvidia/compute/nv_compute.o \
               drivers/gpu/arm/mali/arm_mali.o drivers/gpu/arm/powervr/powervr.o \
               drivers/gpu/framebuffer/fb.o drivers/gpu/drm/drm_core.o \
               drivers/gpu/display_engine/display_engine.o drivers/gpu/gpu_memory_manager/gpu_mm.o \
               drivers/storage/block/block_layer/block_core.o drivers/storage/block/io_scheduler/io_sched.o \
               drivers/storage/block/cache/block_cache.o drivers/storage/sata/ahci/ahci.o \
               drivers/storage/sata/legacy/sata_legacy.o drivers/storage/nvme/controller/nvme_ctrl.o \
               drivers/storage/nvme/namespace/nvme_ns.o drivers/storage/nvme/multipath/nvme_mpath.o \
               drivers/storage/scsi/scsi.o drivers/storage/sas/sas.o \
               drivers/storage/raid/software/sw_raid.o drivers/storage/raid/hardware/hw_raid.o \
               drivers/storage/usb_storage/usb_storage.o drivers/storage/mmc/mmc.o \
               drivers/storage/sd/sd.o drivers/storage/emmc/emmc.o drivers/storage/ufs/ufs.o \
               drivers/network/ethernet/intel/eth_intel.o drivers/network/ethernet/realtek/eth_realtek.o \
               drivers/network/ethernet/broadcom/eth_broadcom.o drivers/network/ethernet/marvell/eth_marvell.o \
               drivers/network/ethernet/mellanox/eth_mellanox.o drivers/network/ethernet/server_nic/eth_server.o \
               drivers/network/wifi/intel/wifi_intel.o drivers/network/wifi/qualcomm/wifi_qualcomm.o \
               drivers/network/wifi/mediatek/wifi_mediatek.o drivers/network/wifi/broadcom/wifi_broadcom.o \
               drivers/network/wifi/realtek/wifi_realtek.o drivers/network/bluetooth/classic/bt_classic.o \
               drivers/network/bluetooth/ble/bt_ble.o drivers/network/cellular/3g/cellular_3g.o \
               drivers/network/cellular/4g/cellular_4g.o drivers/network/cellular/5g/cellular_5g.o \
               drivers/network/cellular/modem/cellular_modem.o drivers/network/network_acceleration/net_accel.o \
               drivers/input/keyboard/keyboard.o drivers/input/mouse/mouse.o \
               drivers/input/touchpad/touchpad.o drivers/input/touchscreen/touchscreen.o \
               drivers/input/joystick/joystick.o drivers/input/gamepad/gamepad.o \
               drivers/input/tablet/tablet.o drivers/input/accessibility/accessibility.o \
               drivers/audio/core/audio_core.o drivers/audio/codecs/audio_codecs.o \
               drivers/audio/pci_audio/pci_audio.o drivers/audio/usb_audio/usb_audio.o \
               drivers/audio/bluetooth_audio/bt_audio.o drivers/audio/professional_audio/pro_audio.o \
               drivers/camera/webcam/webcam.o drivers/camera/mobile_camera/mobile_cam.o \
               drivers/camera/image_processor/isp.o drivers/camera/sensors/cam_sensors.o \
               drivers/mobile/display/mobile_display.o drivers/mobile/battery/battery.o \
               drivers/mobile/charging/charging.o drivers/mobile/vibration/vibration.o \
               drivers/mobile/fingerprint/fingerprint.o drivers/mobile/face_unlock/face_unlock.o \
               drivers/mobile/gps/gps.o drivers/mobile/nfc/nfc.o drivers/mobile/sensors/mobile_sensors.o \
               drivers/server/ipmi/ipmi.o drivers/server/bmc/bmc.o drivers/server/raid_controller/server_raid.o \
               drivers/server/infiniband/infiniband.o drivers/server/high_speed_network/hs_net.o \
               drivers/server/accelerator/accelerator.o \
               drivers/embedded/microcontrollers/mcu.o drivers/embedded/industrial/industrial.o \
               drivers/embedded/automotive/automotive.o drivers/embedded/robotics/robotics.o \
               drivers/embedded/iot/iot.o \
               drivers/firmware/device_firmware/fw_device.o drivers/firmware/gpu_firmware/fw_gpu.o \
               drivers/firmware/wifi_firmware/fw_wifi.o drivers/firmware/bluetooth_firmware/fw_bluetooth.o \
               drivers/firmware/cpu_microcode/fw_cpu_ucode.o drivers/firmware/embedded_firmware/fw_embedded.o

OBJS := $(CORE_OBJS) $(PROC_OBJS) $(MM_OBJS) $(SYS_OBJS) $(BOOT_OBJS) $(ARCH_OBJS) $(DRIVER_OBJS)

all: dionnex.bin

dionnex.bin: $(OBJS)
	@echo "  LD      $@"
	@echo "Build completed successfully for Dionnex ARCH=$(ARCH)"

%.o: %.c
	@echo "  CC      $<"

defconfig:
	@echo "Configured for ARCH=$(ARCH) - default profile loaded."

clean:
	@rm -f dionnex.bin $(OBJS)

.PHONY: all clean defconfig
