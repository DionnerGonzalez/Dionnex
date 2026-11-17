/* ============================================================================
 * Dionnex Monolithic Kernel - DMA Coherent Physical Memory Allocator Header
 * ============================================================================
 */
#ifndef _DIONNEX_DMA_H_
#define _DIONNEX_DMA_H_

#include <types.h>

typedef struct dma_buffer {
    void     *virt;
    uint32_t  phys;
    uint32_t  size;
    uint32_t  pages;
} dma_buffer_t;

int dma_alloc_coherent(uint32_t size, dma_buffer_t *out_buf);
void dma_free_coherent(dma_buffer_t *buf);

#endif /* _DIONNEX_DMA_H_ */
