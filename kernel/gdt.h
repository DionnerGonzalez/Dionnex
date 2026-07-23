/* ============================================================================
 * Dionnex Monolithic Kernel - Global Descriptor Table (GDT) Header
 * ============================================================================
 */
#ifndef _DIONNEX_GDT_H_
#define _DIONNEX_GDT_H_

#include <types.h>

struct gdt_entry_struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

void gdt_init(void);
extern void gdt_flush(uint32_t gdt_ptr);

#endif /* _DIONNEX_GDT_H_ */
