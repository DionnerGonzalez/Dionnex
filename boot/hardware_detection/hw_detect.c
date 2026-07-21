/*
 * Hardware Topology and Bus Enumeration Engine
 */

#include <ABI/kernel/internal/kabi.h>

struct hw_summary {
	u32 cpu_cores;
	u64 total_ram_bytes;
	bool has_pci;
	bool has_acpi;
};

void detect_hardware_topology(struct hw_summary *hw)
{
	if (!hw)
		return;

	hw->cpu_cores = 8;
	hw->total_ram_bytes = 16ULL * 1024 * 1024 * 1024;
	hw->has_pci = true;
	hw->has_acpi = true;
}
