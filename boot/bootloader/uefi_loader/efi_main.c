/*
 * UEFI Boot Loader Handover Module
 */

#include <ABI/kernel/internal/kabi.h>

void efi_handover_init(void)
{
	/* Exit Boot Services and pass ACPI RSDP pointer to DionOS early setup */
}
