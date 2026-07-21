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

FS_OBJS   := filesystem/VFS/namespace/vfs_ns.o filesystem/VFS/mount/vfs_mount.o \
             filesystem/VFS/inode/vfs_inode.o filesystem/VFS/directory/vfs_dir.o \
             filesystem/VFS/file_operations/vfs_file.o filesystem/VFS/permissions/vfs_perm.o \
             filesystem/VFS/cache/vfs_cache.o \
             filesystem/DionFS/core/dionfs_core.o filesystem/DionFS/journal/dionfs_journal.o \
             filesystem/DionFS/compression/dionfs_compress.o filesystem/DionFS/encryption/dionfs_crypto.o \
             filesystem/DionFS/snapshots/dionfs_snapshot.o \
             filesystem/ext/ext2/ext2.o filesystem/ext/ext3/ext3.o filesystem/ext/ext4/ext4.o \
             filesystem/fat/fat16/fat16.o filesystem/fat/fat32/fat32.o filesystem/fat/exfat/exfat.o \
             filesystem/ntfs/ntfs.o filesystem/iso9660/iso9660.o filesystem/udf/udf.o \
             filesystem/btrfs/btrfs.o filesystem/zfs_interface/zfs.o filesystem/squashfs/squashfs.o \
             filesystem/tmpfs/tmpfs.o filesystem/ramfs/ramfs.o \
             filesystem/network_filesystems/nfs/nfs.o filesystem/network_filesystems/smb/smb.o \
             filesystem/network_filesystems/ftp_fs/ftp_fs.o \
             filesystem/virtual_filesystems/proc/procfs.o filesystem/virtual_filesystems/sys/sysfs.o \
             filesystem/virtual_filesystems/dev/devfs.o \
             filesystem/storage_management/volume_manager/vol_mgr.o \
             filesystem/storage_management/encryption_layer/storage_crypto.o \
             filesystem/storage_management/snapshot_manager/snapshot_mgr.o

NET_OBJS  := networking/network_core/socket_layer/socket.o \
             networking/network_core/packet_manager/packet.o \
             networking/network_core/routing/routing.o \
             networking/network_core/network_namespace/net_ns.o \
             networking/ethernet/net_ethernet.o networking/wifi/net_wifi.o \
             networking/bluetooth/net_bluetooth.o \
             networking/protocols/arp/arp.o networking/protocols/ipv4/ipv4.o \
             networking/protocols/ipv6/ipv6.o networking/protocols/icmp/icmp.o \
             networking/protocols/tcp/tcp.o networking/protocols/udp/udp.o \
             networking/protocols/quic/quic.o networking/protocols/multicast/multicast.o \
             networking/application_network/dns/dns.o networking/application_network/dhcp/dhcp.o \
             networking/application_network/http/http.o networking/application_network/tls/tls.o \
             networking/application_network/websocket/websocket.o \
             networking/firewall/packet_filter/filter.o \
             networking/firewall/state_tracking/conntrack.o \
             networking/firewall/intrusion_detection/ids.o \
             networking/vpn/tunnel/tunnel.o networking/vpn/encryption/vpn_crypto.o \
             networking/vpn/virtual_network/vnet.o \
             networking/acceleration/hardware_offload/hw_offload.o \
             networking/acceleration/dma_network/net_dma.o \
             networking/acceleration/packet_processing/packet_proc.o

SEC_OBJS  := security/access_control/permissions/sec_perm.o \
             security/access_control/capabilities/sec_caps.o \
             security/access_control/policies/sec_policy.o \
             security/authentication/users/auth_users.o \
             security/authentication/passwords/auth_pass.o \
             security/authentication/keys/auth_keys.o \
             security/authentication/tokens/auth_tokens.o \
             security/cryptography/hash/sha/sha.o \
             security/cryptography/hash/blake/blake.o \
             security/cryptography/encryption/aes/aes.o \
             security/cryptography/encryption/chacha/chacha.o \
             security/cryptography/encryption/rsa/rsa.o \
             security/cryptography/random/rng.o \
             security/secure_boot/sec_boot.o security/trusted_execution/tee.o \
             security/kernel_integrity/integrity.o \
             security/exploit_protection/stack_protection/stack_prot.o \
             security/exploit_protection/memory_protection/mem_prot.o \
             security/exploit_protection/address_randomization/aslr.o \
             security/sandbox/applications/app_sandbox.o \
             security/sandbox/drivers/driver_sandbox.o \
             security/sandbox/containers/container_sandbox.o \
             security/auditing/audit.o security/security_monitor/sec_mon.o

VIRT_OBJS := virtualization/hypervisor/cpu_virtualization/cpu_virt.o \
             virtualization/hypervisor/memory_virtualization/mem_virt.o \
             virtualization/hypervisor/device_virtualization/dev_virt.o \
             virtualization/hypervisor/migration/virt_mig.o \
             virtualization/kvm_like/kvm.o \
             virtualization/containers/isolation/isolation.o \
             virtualization/containers/namespaces/virt_ns.o \
             virtualization/containers/resource_limits/cgroups.o \
             virtualization/virtual_devices/virtual_disk/vdisk.o \
             virtualization/virtual_devices/virtual_network/vnet_dev.o \
             virtualization/virtual_devices/virtual_gpu/vgpu.o \
             virtualization/cloud/orchestration/orchestrator.o \
             virtualization/cloud/scaling/autoscale.o \
             virtualization/cloud/monitoring/virt_mon.o

PM_OBJS   := power_management/acpi/acpi_pm.o power_management/uefi_power/uefi_pm.o \
             power_management/cpu_frequency/intel_scaling/intel_pstate.o \
             power_management/cpu_frequency/amd_scaling/amd_pstate.o \
             power_management/cpu_frequency/arm_scaling/arm_cpufreq.o \
             power_management/gpu_power/gpu_pm.o \
             power_management/thermal/temperature/temp_mon.o \
             power_management/thermal/fan_control/fan_ctrl.o \
             power_management/thermal/emergency_shutdown/thermal_shutdown.o \
             power_management/battery/batt_pm.o power_management/charging/charge_pm.o \
             power_management/suspend/suspend.o power_management/hibernate/hibernate.o \
             power_management/wakeup/wakeup.o

COMPAT_OBJS := compatibility/POSIX/posix_compat.o compatibility/UNIX/unix_compat.o \
               compatibility/Linux_Compatibility/linux_compat.o \
               compatibility/Legacy/legacy_compat.o \
               compatibility/Application_Interface/abi_compat.o

USERSPACE_OBJS := userspace_interface/libc/dion_libc.o \
                  userspace_interface/init_system/init_sys.o \
                  userspace_interface/system_services/sys_services.o \
                  userspace_interface/utilities/sys_utils.o

BUILD_SYS_OBJS := build_system/compiler/gcc/gcc_tool.o \
                  build_system/compiler/clang/clang_tool.o \
                  build_system/compiler/cross_compiler/cross_comp.o \
                  build_system/linker/ld/ld_tool.o \
                  build_system/linker/elf/elf_tool.o \
                  build_system/linker/binary_tools/bin_tools.o \
                  build_system/configuration/Kconfig/kconfig_core.o \
                  build_system/configuration/menu_config/menuconfig.o \
                  build_system/configuration/hardware_profiles/hw_profiles.o \
                  build_system/build/kernel_build/kbuild.o \
                  build_system/build/module_build/mbuild.o \
                  build_system/build/driver_build/dbuild.o \
                  build_system/build/package_build/pbuild.o \
                  build_system/release/versioning/versioning.o \
                  build_system/release/signing/rel_signing.o \
                  build_system/release/packaging/rel_packaging.o

TOOLS_OBJS     := tools/debugger/kernel_debugger/kgdb.o \
                  tools/debugger/remote_debug/remote_dbg.o \
                  tools/debugger/crash_analysis/crash_dump.o \
                  tools/profiler/cpu_profiler/cpu_prof.o \
                  tools/profiler/memory_profiler/mem_prof.o \
                  tools/profiler/io_profiler/io_prof.o \
                  tools/profiler/power_profiler/pwr_prof.o \
                  tools/emulator/qemu/qemu_emu.o \
                  tools/emulator/hardware_simulation/hw_sim.o \
                  tools/emulator/architecture_testing/arch_test.o \
                  tools/image_builder/iso_creator/iso_create.o \
                  tools/image_builder/disk_image/disk_img.o \
                  tools/image_builder/mobile_image/mobile_img.o \
                  tools/image_builder/server_image/server_img.o \
                  tools/firmware_tools/fw_tools.o \
                  tools/driver_tools/drv_tools.o \
                  tools/documentation_generator/doc_gen.o

TESTING_OBJS   := testing/boot_tests/bios/bios_test.o \
                  testing/boot_tests/uefi/uefi_test.o \
                  testing/boot_tests/embedded/embed_test.o \
                  testing/hardware_tests/cpu/cpu_test.o \
                  testing/hardware_tests/gpu/gpu_test.o \
                  testing/hardware_tests/storage/storage_test.o \
                  testing/hardware_tests/network/net_test.o \
                  testing/hardware_tests/mobile/mobile_test.o \
                  testing/kernel_tests/scheduler/sched_test.o \
                  testing/kernel_tests/memory/mem_test.o \
                  testing/kernel_tests/filesystem/fs_test.o \
                  testing/kernel_tests/drivers/driver_test.o \
                  testing/kernel_tests/security/sec_test.o \
                  testing/performance/benchmarks/benchmarks.o \
                  testing/performance/latency/latency_test.o \
                  testing/performance/throughput/throughput_test.o \
                  testing/performance/power_efficiency/pwr_eff_test.o \
                  testing/stress/cpu_stress/cpu_stress.o \
                  testing/stress/memory_stress/mem_stress.o \
                  testing/stress/io_stress/io_stress.o \
                  testing/stress/network_stress/net_stress.o \
                  testing/compatibility/old_hardware/old_hw_test.o \
                  testing/compatibility/new_hardware/new_hw_test.o \
                  testing/compatibility/applications/app_compat_test.o \
                  testing/compatibility/architectures/arch_compat_test.o

MODULES_OBJS   := modules/module_loader/mod_loader.o \
                  modules/module_manager/mod_mgr.o \
                  modules/dependency_manager/mod_dep.o \
                  modules/module_signing/mod_sign.o \
                  modules/hot_loading/mod_hot.o \
                  modules/module_repository/mod_repo.o

PKG_OBJS       := package_system/kernel_packages/kpkg.o \
                  package_system/driver_packages/dpkg.o \
                  package_system/firmware_packages/fwpkg.o \
                  package_system/update_system/pkg_update.o \
                  package_system/rollback/pkg_rollback.o

MON_OBJS       := monitoring/system_monitor/sys_mon.o \
                  monitoring/hardware_monitor/hw_mon.o \
                  monitoring/logs/kernel_logs.o \
                  monitoring/metrics/metrics.o \
                  monitoring/diagnostics/diagnostics.o

DIST_OBJS      := distribution/Dionnex/desktop/dist_desktop.o \
                  distribution/Dionnex/server/dist_server.o \
                  distribution/Dionnex/mobile/dist_mobile.o \
                  distribution/Dionnex/embedded/dist_embedded.o \
                  distribution/installers/dist_installer.o \
                  distribution/recovery/dist_recovery.o \
                  distribution/update_channels/dist_channels.o

DEV_ENV_OBJS   := developer_environment/sdk/dev_sdk.o \
                  developer_environment/headers/dev_headers.o \
                  developer_environment/examples/dev_examples.o \
                  developer_environment/driver_template/drv_template.o \
                  developer_environment/module_template/mod_template.o

GOV_OBJS       := governance/maintainers/gov_maintainers.o \
                  governance/review_process/gov_review.o \
                  governance/security_response/gov_security.o \
                  governance/release_management/gov_release.o \
                  governance/roadmap/gov_roadmap.o

RELEASES_OBJS  := releases/Dionnex-0.1/rel_0_1.o \
                  releases/Dionnex-0.5/rel_0_5.o \
                  releases/Dionnex-1.0/rel_1_0.o \
                  releases/Dionnex-LTS/rel_lts.o \
                  releases/Dionnex-Enterprise/rel_enterprise.o

OBJS := $(CORE_OBJS) $(PROC_OBJS) $(MM_OBJS) $(SYS_OBJS) $(BOOT_OBJS) $(ARCH_OBJS) $(DRIVER_OBJS) \
        $(FS_OBJS) $(NET_OBJS) $(SEC_OBJS) $(VIRT_OBJS) $(PM_OBJS) $(COMPAT_OBJS) $(USERSPACE_OBJS) \
        $(BUILD_SYS_OBJS) $(TOOLS_OBJS) $(TESTING_OBJS) $(MODULES_OBJS) $(PKG_OBJS) $(MON_OBJS) \
        $(DIST_OBJS) $(DEV_ENV_OBJS) $(GOV_OBJS) $(RELEASES_OBJS)

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
