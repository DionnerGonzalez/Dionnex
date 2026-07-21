/*
 * ELF64 Binary Format Definitions
 */

#ifndef _DIONOS_ELF_H
#define _DIONOS_ELF_H

#include <ABI/kernel/internal/kabi.h>

#define EI_NIDENT 16

typedef struct {
	u8  e_ident[EI_NIDENT];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u64 e_entry;
	u64 e_phoff;
	u64 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} Elf64_Ehdr;

#define EM_X86_64 62
#define EM_AARCH64 183
#define EM_RISCV 243

#endif /* _DIONOS_ELF_H */
