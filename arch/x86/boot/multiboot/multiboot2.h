/*
 * Multiboot2 Specification Header for DionOS x86
 */

#ifndef _MULTIBOOT2_H
#define _MULTIBOOT2_H

#define MULTIBOOT2_HEADER_MAGIC                  0xe85250d6
#define MULTIBOOT2_BOOTLOADER_MAGIC              0x36d76289

struct multiboot_tag {
	unsigned int type;
	unsigned int size;
};

struct multiboot_tag_mmap {
	unsigned int type;
	unsigned int size;
	unsigned int entry_size;
	unsigned int entry_version;
	char entries[0];
};

#endif /* _MULTIBOOT2_H */
