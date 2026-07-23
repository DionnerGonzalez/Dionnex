CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-stack-check -Wall -Wextra -c

all: hello.ko counter.ko sysinfo_mod.ko

hello.ko: hello.c
	$(CC) $(CFLAGS) $< -o $@

counter.ko: counter.c
	$(CC) $(CFLAGS) $< -o $@

sysinfo_mod.ko: sysinfo_mod.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.ko *.o

.PHONY: all clean
