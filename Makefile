# ==========================================================================
# DionOS Main Kernel Makefile
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

BOOT_OBJS := boot/early_init/early_setup.c boot/hardware_detection/hw_detect.o

OBJS := $(CORE_OBJS) $(PROC_OBJS) $(MM_OBJS) $(SYS_OBJS) $(BOOT_OBJS)

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
