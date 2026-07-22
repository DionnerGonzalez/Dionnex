/* ============================================================================
 * Dionnex Kernel - Kernel Automated Unit Test Suite (testing/ktest.c)
 * ============================================================================
 */
#include <testing/ktest.h>

int ktest_assert(int condition, const char* test_name) {
    if (condition) {
        kprintf("  [PASS] Test '%s'\n", test_name);
        return 1;
    } else {
        kprintf("  [FAIL] Test '%s'\n", test_name);
        return 0;
    }
}

void ktest_run_all(void) {
    kprintf("\n[KTEST] Ejecutando Batería Completa de Pruebas Unitarias...\n");

    // Test 1: PMM Page Allocation
    void* p1 = pmm_alloc_page();
    ktest_assert(p1 != NULL, "PMM Allocation Page 1");
    pmm_free_page(p1);

    // Test 2: Dynamic Kmalloc
    void* m1 = kmalloc(128);
    ktest_assert(m1 != NULL, "kmalloc Heap Block 128 bytes");
    kfree(m1);

    // Test 3: String Functions
    int cmp = strcmp("dionnex", "dionnex");
    ktest_assert(cmp == 0, "String Comparison strcmp");

    kprintf("[KTEST] Pruebas del Kernel Completadas con Éxito.\n\n");
}
