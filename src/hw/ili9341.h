#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ILI9341_TFTWIDTH	240
#define ILI9341_TFTHEIGHT	320

#define ILI9341_BLACK		0x0000
#define ILI9341_NAVY		0x000F
#define ILI9341_DARKGREEN	0x03E0
#define ILI9341_DARKCYAN	0x03EF
#define ILI9341_MAROON		0x7800
#define ILI9341_PURPLE		0x780F
#define ILI9341_OLIVE		0x7BE0
#define ILI9341_LIGHTGREY	0xC618
#define ILI9341_DARKGREY	0x7BEF
#define ILI9341_BLUE		0x001F
#define ILI9341_GREEN		0x07E0
#define ILI9341_CYAN		0x07FF
#define ILI9341_RED			0xF800
#define ILI9341_MAGENTA		0xF81F
#define ILI9341_YELLOW		0xFFE0
#define ILI9341_WHITE		0xFFFF
#define ILI9341_ORANGE		0xFD20
#define ILI9341_GREENYELLOW	0xAFE5
#define ILI9341_PINK		0xF81F

void lcd_set_pins(uint16_t dc, uint16_t cs, int16_t rst, uint16_t sck, uint16_t tx);
void lcd_init(void);
void lcd_set_rotation(uint8_t m);
void lcd_fill_screen(uint16_t color);
void lcd_draw_pixel(int16_t x, int16_t y, uint16_t color);
void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void lcd_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color);
void lcd_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color);
void lcd_invert_display(bool invert);
uint16_t lcd_color565(uint8_t r, uint8_t g, uint8_t b);

void lcd_swap_buffers(void);
void lcd_present(void);