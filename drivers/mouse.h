/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Mouse Driver Header (IRQ12)
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_MOUSE_H_
#define _DIONNEX_MOUSE_H_

#include <types.h>

typedef struct mouse_state {
    int32_t x;
    int32_t y;
    uint8_t buttons;
    uint32_t packets;
} mouse_state_t;

void mouse_init(void);
void mouse_get_state(mouse_state_t *out);

#endif /* _DIONNEX_MOUSE_H_ */
