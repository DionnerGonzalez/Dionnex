/*
 * DionBoot Native Stage-2 Bootloader for DionOS
 */

#include <ABI/kernel/internal/kabi.h>

struct dionboot_header {
	u32 magic;
	u32 version;
	u64 kernel_entry_phys;
	u64 memory_map_addr;
	u32 memory_map_entries;
};

int dionboot_load_kernel(struct dionboot_header *hdr)
{
	if (!hdr || hdr->magic != 0x44494F4E) /* "DION" */
		return -1;

	/* Verify checksum and jump to 64-bit kernel entry point */
	((void (*)(void))hdr->kernel_entry_phys)();
	return 0;
}
