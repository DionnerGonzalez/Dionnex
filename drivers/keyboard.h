/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Keyboard Driver Header
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#ifndef _DIONNEX_KEYBOARD_H_
#define _DIONNEX_KEYBOARD_H_

#include <types.h>

#define KEY_SPECIAL_UP    0x80
#define KEY_SPECIAL_DOWN  0x81
#define KEY_SPECIAL_LEFT  0x82
#define KEY_SPECIAL_RIGHT 0x83

void keyboard_init(void);
char keyboard_read(void);
int keyboard_has_input(void);
int keyboard_is_ctrl_pressed(void);
int keyboard_is_alt_pressed(void);

#endif /* _DIONNEX_KEYBOARD_H_ */
