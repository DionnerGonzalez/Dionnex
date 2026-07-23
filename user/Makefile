# Dionnex Ring 3 Userspace Programs Makefile
CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I../include
LD = ld
LDFLAGS = -m elf_i386 -T init.ld -nostdlib

all: init.elf shell_user.elf

init.elf: init.c init.ld
	$(CC) $(CFLAGS) -c init.c -o init.o
	$(LD) $(LDFLAGS) init.o -o init.elf

shell_user.elf: shell_user.c init.ld
	$(CC) $(CFLAGS) -c shell_user.c -o shell_user.o
	$(LD) $(LDFLAGS) shell_user.o -o shell_user.elf

clean:
	rm -f *.o *.elf
