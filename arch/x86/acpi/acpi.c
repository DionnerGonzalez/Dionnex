/*
 * Advanced Configuration and Power Interface (ACPI) Parser
 */

#include <ABI/kernel/internal/kabi.h>

struct rsdp_descriptor {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_address;
} __attribute__((packed));

int acpi_init(void)
{
	/* Locate RSDP and parse MADT for SMP CPU topology */
	return 0;
}
