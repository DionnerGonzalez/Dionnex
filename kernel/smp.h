/* ============================================================================
 * Dionnex Monolithic Kernel - Symmetric Multiprocessing (SMP) Header
 * ============================================================================
 */
#ifndef _DIONNEX_SMP_H_
#define _DIONNEX_SMP_H_

#include <types.h>

#define MAX_CPUS 32

struct mp_floating_pointer {
    char signature[4]; // "_MP_"
    uint32_t phys_addr;
    uint8_t length;
    uint8_t spec_rev;
    uint8_t checksum;
    uint8_t mp_feature1;
    uint8_t mp_feature2;
    uint8_t mp_feature3_5[3];
} __attribute__((packed));

typedef struct mp_floating_pointer mp_floating_pointer_t;

struct mp_config_header {
    char signature[4]; // "PCMP"
    uint16_t length;
    uint8_t spec_rev;
    uint8_t checksum;
    char oem_id[8];
    char product_id[12];
    uint32_t oem_table_ptr;
    uint16_t oem_table_size;
    uint16_t entry_count;
    uint32_t lapic_addr;
    uint16_t extended_table_length;
    uint8_t extended_table_checksum;
    uint8_t reserved;
} __attribute__((packed));

typedef struct mp_config_header mp_config_header_t;

struct smp_cpu_info {
    uint8_t apic_id;
    uint8_t enabled;
    uint8_t is_bsp;
};

typedef struct smp_cpu_info smp_cpu_info_t;

void smp_init(void);
uint32_t smp_get_cpu_count(void);
uint32_t smp_get_current_cpu(void);
int smp_is_bsp(void);
void smp_list_cpus(void);

#endif /* _DIONNEX_SMP_H_ */
