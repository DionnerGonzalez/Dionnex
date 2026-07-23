/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Keyboard Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_KEYBOARD_H_
#define _DIONNEX_KEYBOARD_H_

#include <types.h>

void keyboard_init(void);
char keyboard_read(void);
int keyboard_has_input(void);

#endif /* _DIONNEX_KEYBOARD_H_ */
