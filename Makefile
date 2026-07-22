# ============================================================================
# Dionnex Real OS Kernel - Production Build System Makefile
# ============================================================================

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -fno-pie -no-pie -Wall -Wextra -Iinclude -I. -O2 -g
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES = $(shell find kernel memory process drivers fs loader shell -name "*.c" 2>/dev/null)
ASM_SOURCES = boot.asm arch/x86/switch.asm

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = boot.o arch/x86/switch.o

KERNEL_BIN = kernel.bin
KERNEL_ISO = dionnex_kernel.iso

.PHONY: all clean iso run

all: $(KERNEL_BIN)

boot.o: boot.asm
	$(AS) $(ASFLAGS) $< -o $@

arch/x86/switch.o: arch/x86/switch.asm
	@mkdir -p arch/x86
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	$(LD) $(LDFLAGS) $(ASM_OBJECTS) $(C_OBJECTS) -o $(KERNEL_BIN)
	@echo "SUCCESS: $(KERNEL_BIN) compiled and linked cleanly."

iso: $(KERNEL_BIN)
	@mkdir -p iso_root/boot/grub
	@cp $(KERNEL_BIN) iso_root/boot/kernel.bin
	@echo 'menuentry "Dionnex Real Kernel OS" {' > iso_root/boot/grub/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> iso_root/boot/grub/grub.cfg
	@echo '}' >> iso_root/boot/grub/grub.cfg
	@grub-mkrescue -o $(KERNEL_ISO) iso_root/
	@echo "SUCCESS: ISO created at $(KERNEL_ISO)"

run: iso
	qemu-system-i386 -cdrom $(KERNEL_ISO) -m 512M -serial stdio

clean:
	rm -f $(C_OBJECTS) $(ASM_OBJECTS) $(KERNEL_BIN) $(KERNEL_ISO)
	rm -rf iso_root
