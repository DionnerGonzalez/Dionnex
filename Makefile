# ============================================================================
# Dionnex Monolithic Kernel - Production Makefile
# ============================================================================

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-stack-check -Wall -Wextra -Iinclude

AS = nasm
ASFLAGS = -f elf32

LD = ld
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

C_SOURCES = \
	kernel/main.c \
	kernel/shell.c \
	kernel/printk.c \
	kernel/panic.c \
	kernel/gdt.c \
	kernel/idt.c \
	kernel/timer.c \
	mm/pmm.c \
	mm/vmm.c \
	sched/task.c \
	sched/scheduler.c \
	fs/ramfs.c \
	drivers/vga.c \
	drivers/keyboard.c \
	drivers/pic.c

ASM_SOURCES = \
	boot/boot.asm \
	sched/switch.asm \
	sched/gdt_flush.asm \
	sched/idt_stubs.asm

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

all: $(C_OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o dionnex.bin $(ASM_OBJECTS) $(C_OBJECTS)
	@echo "=== Dionnex Monolithic Kernel built successfully ==="

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
	qemu-system-i386 -kernel dionnex.bin -serial stdio

clean:
	rm -rf $(C_OBJECTS) $(ASM_OBJECTS) dionnex.bin dionnex.iso iso/boot/dionnex.bin

.PHONY: all iso run clean
