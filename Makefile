# ============================================================================
# Dionnex Kernel - Production Build System Makefile
# ============================================================================

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-pie -Wall -Wextra -I. -Iinclude -Ikernel
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T arch/x86_64/linker.ld

C_SOURCES = \
	kernel/init.c \
	kernel/kprintf.c \
	kernel/timer.c \
	kernel/keyboard.c \
	kernel/panic.c \
	kernel/shell.c \
	memory/pmm.c \
	memory/vmm.c \
	memory/kmalloc.c \
	process/task.c \
	process/cfs.c \
	process/elf.c \
	drivers/irq.c \
	drivers/drivers.c \
	syscall/syscall_table.c \
	filesystem/vfs.c \
	filesystem/initramfs.c \
	arch/x86_64/gdt.c \
	arch/x86_64/idt.c \
	arch/x86_64/tss.c \
	networking/net_core.c \
	security/capability.c \
	virtualization/kvm.c \
	power_management/pm.c \
	hardware_abstraction/hal.c \
	driver_framework/device.c \
	performance/perf.c \
	monitoring/telemetry.c \
	userspace_interface/sys_interface.c \
	testing/ktest.c \
	build_system/kbuild.c \
	kernel_api/api.c \
	memory_advanced/numa.c \
	reliability/watchdog.c \
	compatibility/posix.c

ASM_SOURCES = \
	arch/x86_64/boot.asm \
	arch/x86_64/gdt_flush.asm \
	arch/x86_64/idt_flush.asm \
	arch/x86_64/isr_stubs.asm \
	arch/x86_64/switch.asm

C_OBJS = $(C_SOURCES:.c=.o)
ASM_OBJS = $(ASM_SOURCES:.asm=.o)
OBJS = $(ASM_OBJS) $(C_OBJS)

all: dionnex_kernel.bin

dionnex_kernel.bin: $(OBJS)
	@echo "[LD] Linking Dionnex Microkernel Binary image..."
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "[SUCCESS] Kernel built successfully: dionnex_kernel.bin"

%.o: %.c
	@echo "[CC] Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo "[AS] Assembling $<"
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(OBJS) dionnex_kernel.bin dionnex_kernel.iso

.PHONY: all clean
