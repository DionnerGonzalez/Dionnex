# Dionnex Monolithic Kernel v0.1.0 (Genesis)

Dionnex is a 32-bit x86 (i686) monolithic operating system kernel written in C and Assembly under the GPLv2 license.

**Author:** Ihosvanni Dionner Gonzalez  
*Portions adapted from Linux kernel (GPLv2)*

---

## Architecture Overview

Dionnex implements a monolithic architecture providing bare-metal x86 management with low-level execution primitives, Ring 3 userspace isolation, and standard device abstractions.

### Ring 3 Userspace & Process Isolation
- **Ring Transitions & TSS**: Employs a Task State Segment (TSS) at GDT descriptor index 5 (`0x28`) for Ring 3 to Ring 0 stack switching (`esp0`).
- **Process Management**: User processes run with separate 1MB stack pages (`0xBFFFFFFF`), page directory isolation, virtual code base at `0x08048000`, and dynamically managed program breaks (`brk`).
- **ELF32 Executable Loader**: Validates and maps ELF32 executable segments (`PT_LOAD`) directly into process user memory pages with proper permission flags (`VMM_USER`, `VMM_WRITABLE`).
- **InitRD & User Binaries**: Embedded RAM disk holding pre-compiled static Ring 3 user ELF binaries (`init.elf`, `shell_user.elf`).
- **Preemptive Process Scheduler**: Preemptive vruntime process scheduler running alongside kernel tasks.
- **Fault Protection**: Ring 3 CPU exceptions gracefully terminate offending user processes (`process_exit`) without triggering kernel panics.

### CPU Support & Hardware Identification
- **Vendor Detection**: Automatic identification of Intel ("GenuineIntel") and AMD ("AuthenticAMD") processors.
- **CPU Topology & Features**: Enumeration of Family, Model, Stepping, and feature sets (FPU, TSC, MSR, APIC, PAE, SSE, SSE2, SSE3, x2APIC, NX).
- **Virtualization Detection**: Intel VT-x (`VMX`) and AMD-V (`SVM`) feature checks via CPUID and MSR registers (`MSR_IA32_FEATURE_CONTROL`, `MSR_AMD_VM_CR`).
- **Timestamp Counter (TSC)**: Precision frequency calibration against the PIT timer.

### APIC Interrupt Management
- **Local APIC (LAPIC)**: Standard MMIO mapping at `0xFEE00000` with End-Of-Interrupt (EOI) processing, SVR initialization, and LAPIC timer calibration.
- **I/O APIC**: MMIO access at `0xFEC00000` for routing legacy hardware IRQs 0–15 to IDT interrupt vectors 32–47.
- **PIC Legacy Disabling**: Masks legacy 8259 PIC controllers to ensure pure APIC interrupt handling.

### ACPI Power Management & Reset
- **RSDP & RSDT Lookup**: Scans BIOS memory ranges (`0x9FC00`, `0xE0000`) for the Root System Description Pointer (RSDP) and table checksum validation.
- **FADT Table Parsing**: Locates the Fixed ACPI Description Table (FADT) to retrieve PM1a control block addresses (`PM1a_CNT_BLK`).
- **Power Operations**: Native ACPI shutdown (`SLP_TYPa | SLP_EN`), QEMU ACPI fallback ports (`0x604`, `0xB004`), and 8042 keyboard controller soft reboots.

### PCI Bus Enumeration
- **Bus Scanning**: Full scan across 256 buses, 32 slots, and 8 functions per device.
- **Configuration Space**: Standard 32-bit I/O port config mechanism via `0xCF8` (address) and `0xCFC` (data).
- **Device Discovery**: Reads Vendor ID, Device ID, Class/Subclass codes, Revision, IRQ lines, and Base Address Registers (BAR0–BAR5).

### ATA/IDE Storage Driver
- **PIO Mode**: Primary/Secondary controller channels (`0x1F0`/`0x170`) for Master and Slave IDE drives.
- **Identify Command**: Decodes drive string descriptions, serial numbers, and total sector capacities.
- **Block Read/Write**: Polled sector read (`0x20`), sector write (`0x30`), and flush (`0xE7`) operations.

### Virtual Filesystem (VFS) & ProcFS
- **VFS Abstraction Layer**: Tree-structured filesystem hierarchy with standard `read`, `write`, `readdir`, and `finddir` interface pointers.
- **RAMFS Mount**: In-memory filesystem mounted at `/ram` for volatile file read/write operations.
- **ProcFS Mount**: Dynamic virtual filesystem mounted at `/proc` exposing real-time kernel statistics (`/proc/version`, `/proc/cpuinfo`, `/proc/meminfo`, `/proc/uptime`, `/proc/pci`, `/proc/disk`, `/proc/klog`, `/proc/tasks`, `/proc/devices`).

### Serial COM1 Debugging
- **8250 UART Driver**: Configures COM1 (`0x3F8`) at 115200 baud, 8N1, with FIFO enabled.
- **Mirroring**: Dual-output logging to both VGA text console and COM1 serial output.

### Kernel Logging Subsystem (klog)
- **Ring Buffer**: 8192-byte ring buffer storing log entries with tick timestamps and severity log levels (`EMERG`, `ALERT`, `CRIT`, `ERR`, `WARN`, `NOTICE`, `INFO`, `DEBUG`).

---

## Syscall Table Interface (INT 0x80)

| Number | Name | Arguments | Description |
|--------|------|-----------|-------------|
| 1 | `sys_exit` | code | Terminate process |
| 2 | `sys_fork` | - | Clone process image |
| 3 | `sys_read` | fd, buf, count | Read from file descriptor |
| 4 | `sys_write` | fd, buf, count | Write to file descriptor |
| 5 | `sys_open` | path, flags | Open file path |
| 6 | `sys_close` | fd | Close file descriptor |
| 7 | `sys_waitpid` | pid, status | Wait for child process exit |
| 11 | `sys_exec` | path | Execute ELF executable from InitRD/VFS |
| 12 | `sys_sbrk` | increment | Adjust process program break (heap) |
| 18 | `sys_stat` | path, statbuf | Retrieve file status |
| 19 | `sys_lseek` | fd, offset, whence | Reposition read/write file offset |
| 20 | `sys_getpid` | - | Get current process PID |
| 21 | `sys_getppid` | - | Get parent process PID |
| 22 | `sys_kill` | pid, sig | Send signal to target process |
| 23 | `sys_mmap` | addr, len, prot, flags | Allocate and map virtual memory pages |
| 24 | `sys_munmap` | addr, len | Unmap virtual memory pages |
| 54 | `sys_ioctl` | fd, request, arg | Device control requests |
| 78 | `sys_gettimeofday` | tv, tz | Get current Real-Time Clock epoch |
| 122 | `sys_uname` | buf | Get system identification string |
| 162 | `sys_nanosleep` | ms | Sleep process execution for milliseconds |
| 252 | `sys_exit_group` | code | Terminate all threads in process group |

---

## Directory Structure

```
.
├── COPYING
├── Makefile
├── README.md
├── linker.ld
├── boot/
│   └── boot.asm
├── user/
│   ├── init.c
│   ├── init.ld
│   ├── shell_user.c
│   └── Makefile
├── kernel/
│   ├── main.c
│   ├── shell.c
│   ├── shell.h
│   ├── process.c
│   ├── process.h
│   ├── tss.c
│   ├── tss.h
│   ├── elf_loader.c
│   ├── elf_loader.h
│   ├── initrd.c
│   ├── initrd.h
│   ├── scheduler_process.c
│   ├── scheduler_process.h
│   ├── printk.c
│   ├── printk.h
│   ├── panic.c
│   ├── panic.h
│   ├── gdt.c
│   ├── gdt.h
│   ├── idt.c
│   ├── idt.h
│   ├── isr.h
│   ├── timer.c
│   ├── timer.h
│   ├── klog.c
│   ├── klog.h
│   ├── cpuid.c
│   ├── cpuid.h
│   ├── syscall.c
│   ├── syscall.h
│   ├── string.c
│   ├── smp.c
│   ├── smp.h
│   ├── elf.c
│   ├── elf.h
│   ├── module.c
│   ├── module.h
│   ├── debug.c
│   └── debug.h
├── mm/
│   ├── pmm.c
│   ├── pmm.h
│   ├── vmm.c
│   ├── vmm.h
│   ├── heap.c
│   └── heap.h
├── sched/
│   ├── task.c
│   ├── task.h
│   ├── scheduler.c
│   ├── scheduler.h
│   ├── switch.asm
│   ├── tss_flush.asm
│   ├── gdt_flush.asm
│   ├── idt_stubs.asm
│   └── syscall_stub.asm
├── fs/
│   ├── vfs.c
│   ├── vfs.h
│   ├── ramfs.c
│   ├── ramfs.h
│   ├── procfs.c
│   └── procfs.h
├── drivers/
│   ├── vga.c
│   ├── vga.h
│   ├── keyboard.c
│   ├── keyboard.h
│   ├── pic.c
│   ├── pic.h
│   ├── serial.c
│   ├── serial.h
│   ├── rtc.c
│   ├── rtc.h
│   ├── pci.c
│   ├── pci.h
│   ├── ata.c
│   ├── ata.h
│   ├── apic.c
│   ├── apic.h
│   ├── acpi.c
│   ├── acpi.h
│   ├── vesa.c
│   └── vesa.h
└── include/
    ├── types.h
    ├── io.h
    ├── config.h
    └── string.h
```

---

## Build Instructions

### Prerequisites
- GCC cross-compiler or native `gcc` with `-m32` support
- NASM assembler (`nasm`)
- GNU Linker (`ld`) targeting `elf_i386`
- QEMU emulator (`qemu-system-i386`)

### Commands
```bash
# Compile kernel binary and ring 3 userspace ELFs
make

# Run in QEMU emulator with serial terminal output
make run

# Clean binary objects
make clean
```

---

## Interactive Kernel Shell Commands

| Command | Description |
| ------- | ----------- |
| `help` | Displays available kernel shell commands |
| `clear` | Clears the VGA text buffer screen |
| `cpuid` / `cpuinfo` | Displays CPU vendor string, model family, feature flags, and TSC frequency |
| `pci` | Enumerates and prints all detected PCI bus devices |
| `disk` | Displays connected ATA/IDE hard disk drives and capacities |
| `date` | Shows Real-Time Clock (RTC) date, time, and Unix epoch timestamp |
| `dmesg` / `klog` | Dumps the kernel ring buffer logs |
| `lsmod` | Lists loaded kernel modules |
| `smp` | Displays detected SMP CPU cores and topology |
| `vesa` | Runs the VESA VBE graphics display test demo |
| `memstat` | Shows physical memory and dynamic heap diagnostics |
| `backtrace` | Prints current stack frame pointer backtrace |
| `hexdump <addr> [len]` | Hexdump memory contents starting from specified address |
| `ps` / `procs` | Lists active kernel tasks and Ring 3 user processes |
| `kill <pid>` | Terminates target user process by sending SIGKILL |
| `exec <elf>` | Loads and executes specified user ELF binary |
| `initrd` | Lists embedded InitRD RAM disk user binaries |
| `ls <path>` | Lists directory entries from VFS |
| `cat <file>` | Displays file contents from VFS |
| `echo <msg>` | Echoes message or writes output to VFS (`echo msg > /ram/file.txt`) |
| `uptime` | Shows kernel uptime in seconds |
| `reboot` | Reboots the system via ACPI / keyboard controller reset |
| `shutdown` | Powers off system via ACPI / QEMU power port |
| `panic` | Triggers a test kernel panic |

---

## License

Dionnex Monolithic Kernel is distributed under the GNU General Public License v2.0 (GPLv2).
