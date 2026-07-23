/* ============================================================================
 * Dionnex Monolithic Kernel - Init RAM Disk (InitRD) Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_INITRD_H_
#define _DIONNEX_INITRD_H_

#include <types.h>

struct initrd_file_struct {
    char name[64];
    uint8_t *data;
    uint32_t size;
};

typedef struct initrd_file_struct initrd_file_t;

void initrd_init(void);
uint8_t *initrd_get_file(const char *name, uint32_t *size);
void initrd_list(void);

#endif /* _DIONNEX_INITRD_H_ */
