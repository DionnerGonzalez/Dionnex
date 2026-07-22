# Dionnex Kernel - Modern Modular Monolithic Operating System Kernel

Dionnex is a modern, enterprise-grade modular monolithic kernel designed in compliance with ISO C11 and Linux Kernel Coding Standards. It provides unified support across multiple architectures (x86, x86_64, ARM, ARM64, RISC-V, PowerPC, MIPS, SPARC, s390, LoongArch) with hardware-accelerated memory virtualization, pre-emptive fair scheduling, zero-copy IPC, and dynamic kernel module loading.

## Key Features

- **Monolithic Modular Architecture**: Dynamic module loading (`insmod`/`rmmod`) with kernel symbol exports (`EXPORT_SYMBOL`).
- **Multi-Architecture Support**: Native support for x86_64, ARM64, RISC-V 64-bit, and legacy platforms.
- **Advanced Scheduler**: Completely Fair Scheduler (CFS) for desktop/server and Real-Time POSIX SCHED_FIFO/SCHED_RR schedulers.
- **Virtual Memory Management**: Multi-level page tables (4-level & 5-level paging), Slab/Slub allocators, Hugetlb, DMA protection.
- **POSIX System Call Interface**: Comprehensive syscall table for process control, VFS operations, socket networking, and memory mappings.

## Building the Kernel

```bash
# Configure the kernel
make defconfig
make menuconfig

# Compile kernel binary
make -j$(nproc)

# Compile architecture specific target (e.g. x86_64)
make ARCH=x86_64 bzImage

# Compile ARM64 Image
make ARCH=arm64 Image
```

## Directory Hierarchy

- `arch/`: Architecture-specific assembly, boot routines, MMU setup, and CPU drivers.
- `boot/`: Unified bootloader initialization interface, early kernel setup, and initramfs parsers.
- `kernel/`: Core subsystem implementation (init, interrupts, timers, workqueues, ftrace, panic handling).
- `process/`: Task management, thread execution context switches, namespaces, spinlocks, signals.
- `memory/`: Physical memory allocator (PMM), Virtual memory manager (VMM), Slab allocator, DMA.
- `syscall/`: System call routing table and handler functions for filesystem, IPC, and networking.
- `Documentation/`: Comprehensive technical documentation, ABI guidelines, and API manuals.