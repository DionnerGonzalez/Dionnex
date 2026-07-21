# Dionnex Development Roadmap

## Phase 1: Core Modular Infrastructure (Completed)
- [x] Modern C kernel initialization and core logging.
- [x] Multi-arch boot dispatch (x86_64, ARM64, RISC-V).
- [x] Physical and Virtual Memory Allocators.
- [x] Completely Fair Scheduler & Context Switching.

## Phase 2: Virtual File System & Device Drivers (In Progress)
- [ ] VFS layer with Ext4/Btrfs driver integration.
- [ ] PCIe bus enumeration and MSI-X interrupt delivery.
- [ ] USB 3.0 xHCI controller module.

## Phase 3: Hardware Acceleration & Virtualization
- [ ] Hypervisor abstraction layer (KVM compatibility).
- [ ] GPU DRM driver stack for Intel/AMD/NVIDIA.
- [ ] Real-time audio ALSA driver framework.
