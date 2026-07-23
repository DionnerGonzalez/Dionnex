# ============================================================================
# Dionnex Monolithic Kernel - Production Makefile
# Author: Ihosvanni Dionner Gonzalez
# ============================================================================

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-stack-check -Wall -Wextra -Iinclude -I.

AS = nasm
ASFLAGS = -f elf32

LD = ld
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

C_SOURCES = \
	kernel/main.c \
	kernel/string.c \
	kernel/shell.c \
	kernel/printk.c \
	kernel/panic.c \
	kernel/klog.c \
	kernel/cpuid.c \
	kernel/syscall.c \
	kernel/elf.c \
	kernel/module.c \
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
	mm/pmm.c \
	mm/vmm.c \
	mm/heap.c \
	sched/task.c \
	sched/scheduler.c \
	fs/vfs.c \
	fs/ramfs.c \
	fs/procfs.c \
	drivers/vga.c \
	drivers/keyboard.c \
	drivers/pic.c \
	drivers/serial.c \
	drivers/rtc.c \
	drivers/pci.c \
	drivers/ata.c \
	drivers/apic.c \
	drivers/acpi.c \
	drivers/vesa.c

ASM_SOURCES = \
	boot/boot.asm \
	sched/switch.asm \
	sched/tss_flush.asm \
	sched/gdt_flush.asm \
	sched/idt_stubs.asm \
	sched/syscall_stub.asm

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
INITRD_OBJECTS = initrd_init.o initrd_shell.o

all: user initrd $(C_OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o dionnex.bin $(ASM_OBJECTS) $(C_OBJECTS) $(INITRD_OBJECTS)
	@echo "=== Dionnex Monolithic Kernel built successfully with Ring 3 Userspace ==="

user:
	$(MAKE) -C user

initrd: user
	objcopy -I binary -O elf32-i386 -B i386 user/init.elf initrd_init.o --rename-section .data=.rodata,alloc,load,readonly,data,contents
	objcopy -I binary -O elf32-i386 -B i386 user/shell_user.elf initrd_shell.o --rename-section .data=.rodata,alloc,load,readonly,data,contents

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

clean:
	$(MAKE) -C user clean
	rm -rf $(C_OBJECTS) $(ASM_OBJECTS) $(INITRD_OBJECTS) dionnex.bin dionnex.iso iso/boot/dionnex.bin

.PHONY: all user initrd iso run clean
