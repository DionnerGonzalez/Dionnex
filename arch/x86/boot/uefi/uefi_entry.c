/*
 * UEFI Bootloader Handover Routine for DionOS x86
 */

#include <ABI/kernel/internal/kabi.h>

struct efi_system_table {
	u64 signature;
	u32 revision;
	u32 header_size;
	u32 crc32;
	u32 reserved;
};

int efi_main(void *ImageHandle, struct efi_system_table *SystemTable)
{
	if (!SystemTable)
		return -1;

	/* Initialize early framebuffer and hand off to DionOS kernel entry */
	return 0;
}
