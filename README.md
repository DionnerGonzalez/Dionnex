# Dionnex Monolithic Kernel

Dionnex is an x86 32-bit monolithic kernel written in C and Assembly. It features a Multiboot compliant boot sequence, GDT/IDT management, a Physical Memory Manager (PMM), Virtual Memory Manager (VMM), a preemptive scheduler, RAM file system (RAMFS), PS/2 keyboard driver, PIT timer, VGA text driver, and an interactive kernel shell.

## Repository Structure

```
.
├── Makefile
├── README.md
├── linker.ld
├── boot/
│   └── boot.asm
├── kernel/
│   ├── main.c
│   ├── shell.c
│   ├── shell.h
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
│   └── timer.h
├── mm/
│   ├── pmm.c
│   ├── pmm.h
│   ├── vmm.c
│   └── vmm.h
├── sched/
│   ├── task.c
│   ├── task.h
│   ├── scheduler.c
│   ├── scheduler.h
│   ├── switch.asm
│   ├── gdt_flush.asm
│   └── idt_stubs.asm
├── fs/
│   ├── ramfs.c
│   └── ramfs.h
├── drivers/
│   ├── vga.c
│   ├── vga.h
│   ├── keyboard.c
│   ├── keyboard.h
│   ├── pic.c
│   └── pic.h
├── include/
│   ├── types.h
│   └── io.h
└── iso/
    └── boot/
        └── grub/
            └── grub.cfg
```

## System Requirements

- **Compiler**: `gcc` (with 32-bit target support `-m32`)
- **Assembler**: `nasm`
- **Linker**: `ld` (with `elf_i386` emulation support)
- **Emulator**: `qemu-system-i386`
- **ISO Builder**: `grub-mkrescue` / `xorriso` (optional for `.iso`)

## Building & Running

### Build Kernel Binary
```bash
make
```

### Run in QEMU
```bash
make run
```

### Build Bootable ISO Image
```bash
make iso
```

### Clean Build Artifacts
```bash
make clean
```

## Shell Commands

| Command | Description |
| ------- | ----------- |
| `help` | Displays available shell commands |
| `clear` | Clears the VGA text buffer screen |
| `meminfo` | Displays total, free, and used physical memory |
| `ps` | Lists running tasks managed by the scheduler |
| `ls` | Lists files stored in the RAM file system |
| `touch <file>` | Creates a new empty file in RAMFS |
| `cat <file>` | Reads and displays the contents of a file |
| `echo <msg>` | Echoes text or redirects text to a file (`echo hello > file.txt`) |
| `uptime` | Shows the system uptime in seconds |
| `reboot` | Triggers a hardware system reset |
| `shutdown` | Powers off the QEMU emulator |
| `panic` | Triggers a kernel panic test |

## License

GPLv2
