/* ============================================================================
 * Dionnex Monolithic Kernel - Serial COM1 Port Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_SERIAL_H_
#define _DIONNEX_SERIAL_H_

#include <types.h>

void serial_init(void);
int serial_is_transmit_empty(void);
void serial_putchar(char c);
void serial_write(const char *s);
int serial_received(void);
char serial_getchar(void);
int serial_has_input(void);

#endif /* _DIONNEX_SERIAL_H_ */
