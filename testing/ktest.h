/* ============================================================================
 * Dionnex Kernel - Kernel Automated Unit Testing (testing/ktest.h)
 * ============================================================================
 */
#ifndef _DIONNEX_TESTING_KTEST_H
#define _DIONNEX_TESTING_KTEST_H

#include <kernel/kernel.h>

void ktest_run_all(void);
int ktest_assert(int condition, const char* test_name);

#endif /* _DIONNEX_TESTING_KTEST_H */
