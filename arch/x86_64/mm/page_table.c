/*
 * 64-Bit x86_64 4-Level and 5-Level Page Table Management
 */

#include <ABI/kernel/internal/kabi.h>

#define PML4_INDEX(v) (((v) >> 39) & 0x1FF)
#define PDPT_INDEX(v) (((v) >> 30) & 0x1FF)
#define PD_INDEX(v)   (((v) >> 21) & 0x1FF)
#define PT_INDEX(v)   (((v) >> 12) & 0x1FF)

struct pml4e { u64 entry; };
struct pdpte { u64 entry; };
struct pde   { u64 entry; };
struct pte   { u64 entry; };

void x86_64_map_page(u64 *pml4, virt_addr_t vaddr, phys_addr_t paddr, u64 flags)
{
	u64 pml4_idx = PML4_INDEX(vaddr);
	u64 pt_idx   = PT_INDEX(vaddr);

	/* Map physical frame paddr to virtual address vaddr in page table */
	(void)pml4;
	(void)pml4_idx;
	(void)pt_idx;
	(void)paddr;
	(void)flags;
}
