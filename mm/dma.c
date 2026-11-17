/* ============================================================================
 * Dionnex Monolithic Kernel - DMA Coherent Physical Memory Allocator
 * ============================================================================
 */
#include <mm/dma.h>
#include <mm/pmm.h>
#include <string.h>

int dma_alloc_coherent(uint32_t size, dma_buffer_t *out_buf) {
    if (!out_buf || size == 0) return -1;

    uint32_t pages = (size + PAGE_SIZE - 1U) / PAGE_SIZE;
    uint32_t phys = pmm_alloc_low_pages(pages);
    if (phys == 0) return -1;

    /* Low pages (< 64MB) are identity-mapped in kernel address space */
    memset((void*)phys, 0, pages * PAGE_SIZE);

    out_buf->virt = (void*)phys;
    out_buf->phys = phys;
    out_buf->size = pages * PAGE_SIZE;
    out_buf->pages = pages;
    return 0;
}

void dma_free_coherent(dma_buffer_t *buf) {
    if (!buf || buf->phys == 0 || buf->pages == 0) return;
    pmm_free_pages(buf->phys, buf->pages);
    buf->virt = NULL;
    buf->phys = 0;
    buf->size = 0;
    buf->pages = 0;
}
