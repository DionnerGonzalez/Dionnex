/* ============================================================================
 * Dionnex Monolithic Kernel - ELF32 Executable Loader Facade Header
 * ============================================================================
 */
#ifndef _DIONNEX_ELF_H_
#define _DIONNEX_ELF_H_

#include <types.h>
#include <kernel/elf_loader.h>

void elf_init(void);
int elf_load(const uint8_t *data, uint32_t size);
int elf_load_file(const char *path);

#endif /* _DIONNEX_ELF_H_ */
