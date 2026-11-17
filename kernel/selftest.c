/* ============================================================================
 * Dionnex Monolithic Kernel - In-Kernel Subsystem Selftest Suite
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/selftest.h>
#include <kernel/printk.h>
#include <kernel/sync.h>
#include <kernel/module_loader.h>
#include <kernel/initrd.h>
#include <kernel/process.h>
#include <dionnex_rust.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <mm/dma.h>
#include <fs/vfs.h>
#include <fs/pipe.h>
#include <string.h>

static int test_pmm_vmm_heap_dma(void) {
    uint32_t free_before = pmm_get_free_count();
    uint32_t p1 = pmm_alloc_page();
    uint32_t p_contig = pmm_alloc_pages(4);
    if (!p1 || !p_contig) return -1;

    pmm_free_page(p1);
    pmm_free_pages(p_contig, 4);
    if (pmm_get_free_count() != free_before) return -2;

    void *ptr = kmalloc_aligned(1024, 64);
    if (!ptr || (((uint32_t)(uintptr_t)ptr) & 63U) != 0) return -3;
    memset(ptr, 0x5A, 1024);
    kfree(ptr);
    if (heap_validate() != 0) return -4;

    dma_buffer_t dma_buf;
    if (dma_alloc_coherent(4096U, &dma_buf) != 0 ||
        !dma_buf.virt || !dma_buf.phys || (dma_buf.phys & 0xFFFU) != 0) {
        return -5;
    }
    dma_free_coherent(&dma_buf);

    page_directory_t *pd = vmm_create_address_space();
    if (!pd) return -6;
    uint32_t upage = pmm_alloc_page();
    vmm_zero_phys_page(upage);
    vmm_map_page_in(pd, 0x08050000U, upage, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
    int ok_user = vmm_validate_user_range(pd, 0x08050000U, 256, 1);
    int rej_kern = vmm_validate_user_range(pd, 0x00100000U, 256, 0);
    vmm_destroy_address_space(pd);
    if (!ok_user || rej_kern) return -7;
    if (pmm_get_free_count() < free_before - 16U) return -8;
    return 0;
}

static int test_sync_and_pipe(void) {
    spinlock_t sp;
    spinlock_init(&sp);
    uint32_t f = spin_lock_irqsave(&sp);
    if (!sp.locked) return -1;
    spin_unlock_irqrestore(&sp, f);
    if (sp.locked) return -2;

    semaphore_t sem;
    sem_init(&sem, 1);
    if (sem_trywait(&sem) != 1) return -3;
    if (sem_trywait(&sem) != 0) return -4;
    sem_post(&sem);

    vfs_node_t *rnode = NULL, *wnode = NULL;
    if (pipe_create(&rnode, &wnode) != 0) return -5;
    const char *msg = "DionnexIPC";
    int w = vfs_write(wnode, 0, 10, (const uint8_t*)msg);
    uint8_t rbuf[16];
    memset(rbuf, 0, sizeof(rbuf));
    int r = vfs_read(rnode, 0, 10, rbuf);
    vfs_close(rnode);
    vfs_close(wnode);
    if (w != 10 || r != 10 || memcmp(rbuf, msg, 10) != 0) return -6;
    return 0;
}

static int test_devfs_and_ramfs(void) {
    vfs_node_t *zn = vfs_open("/dev/zero");
    if (!zn) return -1;
    uint8_t buf[16];
    memset(buf, 0xAA, sizeof(buf));
    int n = vfs_read(zn, 0, 16, buf);
    vfs_close(zn);
    if (n != 16) return -2;
    for (int i = 0; i < 16; i++) {
        if (buf[i] != 0) return -3;
    }

    vfs_node_t *nulln = vfs_open("/dev/null");
    if (!nulln) return -4;
    int wn = vfs_write(nulln, 0, 16, buf);
    vfs_close(nulln);
    if (wn != 16) return -5;
    return 0;
}

static int test_module_cycle(void) {
    uint32_t sz = 0;
    uint8_t *data = initrd_get_file("sysinfo_mod.ko", &sz);
    if (!data || sz == 0) return -1;
    if (module_load_binary("sysinfo_mod", data, sz) != 0) return -2;
    if (!module_find_loaded("sysinfo_mod")) return -3;
    if (module_unload_binary("sysinfo_mod") != 0) return -4;
    if (module_find_loaded("sysinfo_mod") != NULL) return -5;
    return 0;
}

int kernel_run_selftests(void) {
    int failures = 0;
    int r;

    printk("=== DIONNEX KERNEL SELFTEST SUITE ===\n");

    r = test_pmm_vmm_heap_dma();
    printk("[TEST] PMM/VMM/Heap/DMA isolation : %s (rc=%d)\n", r == 0 ? "PASS" : "FAIL", r);
    if (r != 0) failures++;

    r = test_sync_and_pipe();
    printk("[TEST] Sync locks & Pipe IPC      : %s (rc=%d)\n", r == 0 ? "PASS" : "FAIL", r);
    if (r != 0) failures++;

    r = test_devfs_and_ramfs();
    printk("[TEST] VFS /dev/zero & /dev/null  : %s (rc=%d)\n", r == 0 ? "PASS" : "FAIL", r);
    if (r != 0) failures++;

    r = test_module_cycle();
    printk("[TEST] ELF .ko load/unload cycle  : %s (rc=%d)\n", r == 0 ? "PASS" : "FAIL", r);
    if (r != 0) failures++;

    r = dionnex_rust_selftest();
    printk("[TEST] Rust no_std FFI selftest   : %s (rc=%d)\n", r == 0 ? "PASS" : "FAIL", r);
    if (r != 0) failures++;

    printk("=== SELFTEST SUMMARY: %d failure(s) ===\n", failures);
    return failures;
}
