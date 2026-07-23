/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Debug & Memory Diagnostic Header
 * ============================================================================
 */
#ifndef _DIONNEX_DEBUG_H_
#define _DIONNEX_DEBUG_H_

#include <types.h>

void debug_mem_stats(void);
void debug_dump_stack(uint32_t count);
void debug_hexdump(const void *addr, uint32_t len);
void debug_page_table_info(void);

#endif /* _DIONNEX_DEBUG_H_ */
