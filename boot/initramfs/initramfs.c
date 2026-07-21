/*
 * Early CPIO Initramfs Unpacker
 */

#include <ABI/kernel/internal/kabi.h>

int unpack_initramfs(const u8 *cpio_data, size_t size)
{
	if (!cpio_data || size == 0)
		return -1;

	/* Unpack initial root filesystem into VFS RAM disk */
	return 0;
}
