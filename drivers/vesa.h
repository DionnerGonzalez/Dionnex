/* ============================================================================
 * Dionnex Monolithic Kernel - VESA VBE Framebuffer Driver Header
 * ============================================================================
 */
#ifndef _DIONNEX_VESA_H_
#define _DIONNEX_VESA_H_

#include <types.h>

struct vesa_mode_info {
    uint16_t attributes;
    uint8_t  winA, winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint32_t win_func;
    uint16_t bytes_per_scanline;
    uint16_t width;
    uint16_t height;
    uint8_t  x_char, y_char;
    uint8_t  planes;
    uint8_t  bpp;
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;
    uint8_t  red_mask, red_position;
    uint8_t  green_mask, green_position;
    uint8_t  blue_mask, blue_position;
    uint8_t  reserved_mask, reserved_position;
    uint8_t  direct_color_attributes;
    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed));

typedef struct vesa_mode_info vesa_mode_info_t;

void vesa_init(void *mb_info);
int vesa_is_enabled(void);
uint32_t vesa_get_width(void);
uint32_t vesa_get_height(void);
uint32_t vesa_get_bpp(void);

void vesa_putpixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t vesa_getpixel(uint32_t x, uint32_t y);
void vesa_clear(uint32_t color);
void vesa_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vesa_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vesa_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void vesa_draw_circle(int xc, int yc, int r, uint32_t color);
void vesa_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void vesa_draw_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg);
void vesa_test_demo(void);

#endif /* _DIONNEX_VESA_H_ */
