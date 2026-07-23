/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Panic & Assertions Header
 * ============================================================================
 */
#ifndef _DIONNEX_PANIC_H_
#define _DIONNEX_PANIC_H_

#include <types.h>

void kernel_panic(const char *msg);
void kernel_assert(int condition, const char *file, int line, const char *expr);

#define KASSERT(cond) kernel_assert((cond), __FILE__, __LINE__, #cond)
#define ASSERT(cond)  KASSERT(cond)

#endif /* _DIONNEX_PANIC_H_ */
