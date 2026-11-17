# ============================================================================
# Dionnex Monolithic Kernel - Production Makefile
# Author: Ihosvanni Dionner Gonzalez
# ============================================================================

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-stack-check -fno-pic -fno-pie -Wall -Wextra -Iinclude -I.

AS = nasm
ASFLAGS = -f elf32

LD = ld
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib -z noexecstack

C_SOURCES = \
	kernel/main.c \
	kernel/string.c \
	kernel/shell.c \
	kernel/printk.c \
	kernel/panic.c \
	kernel/klog.c \
	kernel/cpuid.c \
	kernel/syscall.c \
	kernel/sync.c \
	kernel/elf.c \
	kernel/module.c \
	kernel/module_loader.c \
	kernel/module_symbols.c \
	kernel/debug.c \
	kernel/smp.c \
	kernel/gdt.c \
	kernel/idt.c \
	kernel/timer.c \
	kernel/process.c \
	kernel/tss.c \
	kernel/elf_loader.c \
	kernel/initrd.c \
	kernel/scheduler_process.c \
	kernel/selftest.c \
	mm/pmm.c \
	mm/vmm.c \
	mm/heap.c \
	mm/dma.c \
	sched/task.c \
	sched/scheduler.c \
	fs/vfs.c \
	fs/ramfs.c \
	fs/procfs.c \
	fs/pipe.c \
	fs/devfs.c \
	fs/fat32.c \
	drivers/vga.c \
	drivers/keyboard.c \
	drivers/mouse.c \
	drivers/pic.c \
	drivers/serial.c \
	drivers/rtc.c \
	drivers/pci.c \
	drivers/pcie.c \
	drivers/hpet.c \
	drivers/ata.c \
	drivers/ahci.c \
	drivers/nvme.c \
	drivers/usb/usb_core.c \
	drivers/usb/uhci.c \
	drivers/usb/ohci.c \
	drivers/usb/ehci.c \
	drivers/usb/xhci.c \
	drivers/apic.c \
	drivers/acpi.c \
	drivers/vesa.c

ASM_SOURCES = \
	boot/boot.asm \
	boot/ap_trampoline.asm \
	sched/switch.asm \
	sched/tss_flush.asm \
	sched/gdt_flush.asm \
	sched/idt_stubs.asm \
	sched/syscall_stub.asm

RUST_TARGET = i586-unknown-linux-gnu
RUST_LIB = rust/target/$(RUST_TARGET)/release/libdionnex_rust.a

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
INITRD_OBJECTS = initrd_init.o initrd_shell.o initrd_hello.o initrd_counter.o initrd_sysinfo.o

all: rust user modules initrd $(C_OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o dionnex.bin $(ASM_OBJECTS) $(C_OBJECTS) $(INITRD_OBJECTS) $(RUST_LIB)
	@echo "=== Dionnex Monolithic Kernel built successfully (C + NASM + Rust) ==="

rust:
	PATH="/root/.cargo/bin:$(PATH)" cargo build --manifest-path rust/Cargo.toml --release --target $(RUST_TARGET)

user:
	$(MAKE) -C user

modules:
	$(MAKE) -C modules

initrd: user modules
	objcopy -I binary -O elf32-i386 -B i386 user/init.elf initrd_init.o --rename-section .data=.rodata,alloc,load,readonly,data,contents --redefine-sym _binary_user_init_elf_start=_binary_init_elf_start --redefine-sym _binary_user_init_elf_end=_binary_init_elf_end
	objcopy -I binary -O elf32-i386 -B i386 user/shell_user.elf initrd_shell.o --rename-section .data=.rodata,alloc,load,readonly,data,contents --redefine-sym _binary_user_shell_user_elf_start=_binary_shell_user_elf_start --redefine-sym _binary_user_shell_user_elf_end=_binary_shell_user_elf_end
	objcopy -I binary -O elf32-i386 -B i386 modules/hello.ko initrd_hello.o --rename-section .data=.rodata,alloc,load,readonly,data,contents --redefine-sym _binary_modules_hello_ko_start=_binary_hello_ko_start --redefine-sym _binary_modules_hello_ko_end=_binary_hello_ko_end
	objcopy -I binary -O elf32-i386 -B i386 modules/counter.ko initrd_counter.o --rename-section .data=.rodata,alloc,load,readonly,data,contents --redefine-sym _binary_modules_counter_ko_start=_binary_counter_ko_start --redefine-sym _binary_modules_counter_ko_end=_binary_counter_ko_end
	objcopy -I binary -O elf32-i386 -B i386 modules/sysinfo_mod.ko initrd_sysinfo.o --rename-section .data=.rodata,alloc,load,readonly,data,contents --redefine-sym _binary_modules_sysinfo_mod_ko_start=_binary_sysinfo_mod_ko_start --redefine-sym _binary_modules_sysinfo_mod_ko_end=_binary_sysinfo_mod_ko_end

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

iso: all
	mkdir -p iso/boot/grub
	cp dionnex.bin iso/boot/
	cp iso/boot/grub/grub.cfg iso/boot/grub/ 2>/dev/null || true
	grub-mkrescue -o dionnex.iso iso/

run: all
	qemu-system-i386 -kernel dionnex.bin -serial stdio -m 256M

test-host: user modules
	PATH="/root/.cargo/bin:$(PATH)" cargo build --manifest-path rust/Cargo.toml --release --target x86_64-unknown-linux-gnu
	$(CC) -Wall -Wextra -Werror tests/test_host.c rust/target/x86_64-unknown-linux-gnu/release/libdionnex_rust.a -lpthread -ldl -o tests/test_host
	./tests/test_host

test: iso test-host
	PATH="/root/.cargo/bin:$(PATH)" cargo test --manifest-path rust/Cargo.toml --target x86_64-unknown-linux-gnu
	python3 scripts/test_qemu.py

clean:
	$(MAKE) -C user clean
	$(MAKE) -C modules clean
	rm -rf $(C_OBJECTS) $(ASM_OBJECTS) $(INITRD_OBJECTS) dionnex.bin dionnex.iso iso/boot/dionnex.bin tests/test_host

.PHONY: all rust user modules initrd iso run test-host test clean
