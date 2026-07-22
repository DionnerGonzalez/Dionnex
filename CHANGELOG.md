# DionOS Kernel Changelog

## [1.0.0-RELEASE] - 2026-07-21

### Core Subsystem
- Added modular kernel loader with symbol table export (`EXPORT_SYMBOL`).
- Implemented Kernel Panic framework with backtrace symbol resolution.
- Integrated high-resolution timers and kernel workqueues.

### Architecture Support
- Complete x86_64 4-level page table initialization with APIC driver.
- ARM64 Exception Level 1 (EL1) vector table setup and GIC interrupt routing.
- Initial RISC-V 64-bit Sv39 paging and SBI extension support.

### Memory Management
- Dual physical memory page frame allocator (PMM) with buddy system algorithm.
- SLAB/SLUB cache allocator for kernel objects (`kmem_cache_create`, `kmalloc`).
- Direct Memory Access (DMA) bounce buffer allocation layer.

### Scheduler & Syscalls
- Completely Fair Scheduler (CFS) with vruntime calculations.
- Comprehensive POSIX system call dispatch engine handling `read`, `write`, `fork`, `execve`, `mmap`.
