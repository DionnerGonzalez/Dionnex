/* ============================================================================
 * Dionnex Monolithic Kernel - VGA Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_VGA_H_
#define _DIONNEX_VGA_H_

#include <types.h>

void vga_init(void);
void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c);
void vga_write(const char *s);
void vga_write_hex(uint32_t val);
void vga_write_dec(uint32_t val);
void vga_update_cursor(void);

#endif /* _DIONNEX_VGA_H_ */
