#include <runtime/canvas.h>

#include <third_party/font8x8_basic.h>

RT_Canvas rt_canvas_make(Arena* arena, uint16_t width, uint16_t height)
{
	RT_Canvas c = {};

	c.width = width;
	c.height = height;

	#if !defined PICO_RP2040
		c.back_buffer = arena_push_array(arena, uint8_t, c.width * c.height);
		c.frame_buffer = arena_push_array(arena, uint8_t, c.width * c.height);
	#endif
	
	c.back_color = 0x00;
	c.fore_color = 0xff;

	c.line_width = 1;

	rt_canvas_clear(&c);

	return c;
}

#if defined PICO_RP2040
	static uint16_t r3g3b2_to_r5g6b5(uint8_t color)
	{
		uint8_t b2 = (color >> 6) & 0x03;
		uint8_t g3 = (color >> 3) & 0x07;
		uint8_t r3 = (color >> 0) & 0x07;

		uint8_t r5 = (r3 << 2) | (r3 >> 1);
		uint8_t g6 = (g3 << 3) | g3;
		uint8_t b5 = (b2 << 3) | (b2 << 1) | (b2 >> 1);

		return ((uint16_t)r5 << 11) | ((uint16_t)g6 << 5) | (uint16_t)b5;
	}
#endif

void rt_canvas_clear(RT_Canvas* c)
{
	#if defined PICO_RP2040
		lcd_fill_screen(r3g3b2_to_r5g6b5(c->back_color));
	#else
		__builtin_memset(c->back_buffer, c->back_color, c->width * c->height);
	#endif
}

void rt_canvas_present(RT_Canvas* c)
{
	#if !defined PICO_RP2040
		__builtin_memcpy(c->frame_buffer, c->back_buffer, c->width * c->height);
	#endif
}

void rt_canvas_line(RT_Canvas* c, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	int16_t dx = __builtin_abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -__builtin_abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t error = dx + dy;

	for (;;) 
	{
		rt_canvas_put(c, x0, y0);

		int16_t e2 = 2 * error;
	
		if (e2 >= dy)
		{
			if (x0 == x1)
			{
				break;
			}

			error += dy;
			x0 += sx;
		}

		if (e2 <= dx)
		{
			if (y0 == y1)
			{
				break;
			}

			error += dx;
			y0 += sy;
		}
	}
}

void rt_canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h)
{
	#if defined PICO_RP2040
		const uint16_t cc = r3g3b2_to_r5g6b5(c->fore_color);

		lcd_fill_rect(x, y, w, 1, cc);
		lcd_fill_rect(x, y + h, w, 1, cc);
		lcd_fill_rect(x, y + 1, 1, h - 1, cc);
		lcd_fill_rect(x + w - 1, y + 1, 1, h - 1, cc);
	#else
		for (uint16_t i = 0; i < c->line_width; i++)
		{
			rt_canvas_fill_rect(c, x, y, w, 1);
			rt_canvas_fill_rect(c, x, y + h, w, 1);

			rt_canvas_fill_rect(c, x, y + 1, 1, h - 1);
			rt_canvas_fill_rect(c, x + w - 1, y + 1, 1, h - 1);
		
			x--, y--;
			w += 2, h += 2;
		}
	#endif
}

void rt_canvas_fill_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h)
{
	#if defined PICO_RP2040
		const uint16_t cc = r3g3b2_to_r5g6b5(c->fore_color);

		lcd_fill_rect(x, y, w, h, cc);
	#else
		for (int16_t i = x; i < x + w; i++) 
		{
			for (int16_t j = y; j < y + h; j++) 
			{
				rt_canvas_put(c, i, j);
			}
		}
	#endif
}

void rt_canvas_put(RT_Canvas* c, int16_t x, int16_t y)
{
	#if defined PICO_RP2040
		const uint16_t cc = r3g3b2_to_r5g6b5(c->fore_color);

		lcd_draw_pixel(x, y, cc);
	#else
		if (x >= 0 && y >= 0 && x < c->width && y < c->height)
		{
			c->back_buffer[x + y * c->width] = c->fore_color;
		}
	#endif
}

void rt_canvas_circle(RT_Canvas* c, int16_t cx, int16_t cy, int16_t r)
{
	for (uint16_t i = c->line_width; i > 0; i--)
	{
		int16_t x = 0;
		int16_t y = r;
		int16_t p = 1 - r;
	
		while (x <= y) 
		{
			rt_canvas_put(c, cx + x, cy - y);
			rt_canvas_put(c, cx - x, cy - y);
			rt_canvas_put(c, cx + x, cy + y);
			rt_canvas_put(c, cx - x, cy + y);
	
			rt_canvas_put(c, cx + y, cy - x);
			rt_canvas_put(c, cx - y, cy - x);
			rt_canvas_put(c, cx + y, cy + x);
			rt_canvas_put(c, cx - y, cy + x);
			
			x++;
	
			if (p <= 0) 
			{
				p += 2 * x + 1;
			} 
			else 
			{
				y--;
				p += 2 * (x - y) + 1;
			} 
		}

		r++;
	}
}

void rt_canvas_fill_circle(RT_Canvas* c, int16_t cx, int16_t cy, int16_t r)
{
	int16_t x = 0;
	int16_t y = r;
	int16_t p = 1 - r;

	while (x <= y) 
	{
		rt_canvas_fill_rect(c, cx - x, cy + y, x * 2, 1);
		rt_canvas_fill_rect(c, cx - x, cy - y, x * 2, 1);

		if (x != y) 
		{
			rt_canvas_fill_rect(c, cx - y, cy + x, y * 2, 1);
			rt_canvas_fill_rect(c, cx - y, cy - x, y * 2, 1);
		}

		x++;

		if (p <= 0) 
		{
			p += 2 * x + 1;
		} 
		else 
		{
			y--;
			p += 2 * (x - y) + 1;
		}
	}
}

void rt_canvas_text(RT_Canvas* c, int16_t x, int16_t y, int16_t s, String text)
{
	uint16_t px = 0;
	uint16_t py = 0;

	for (size_t i = 0; i < text.length; i++) 
	{
		if (text.buffer[i] == ' ')
		{
			px++;
			continue;
		}

		if (text.buffer[i] == '\t')
		{
			px += 3;
			continue;
		}

		if (text.buffer[i] == '\n')
		{
			px = 0;
			py++;

			continue;
		}

		for (uint8_t tx = 0; tx < 8; tx++)
		{
			for (uint8_t ty = 0; ty < 8; ty++)
			{
				if (!((font8x8_basic[text.buffer[i]][ty] >> tx) & 1))
				{
					continue;
				}

				rt_canvas_fill_rect(c, x + (px * 8 * s) + (tx * s), y + (py * 8 * s) + (ty * s), s, s);
			}
		}

		px++;
	}
}

uint8_t rt_canvas_pack_color(uint8_t r, uint8_t g, uint8_t b)
{
	return (CLAMP(r, 0, 7) << 5) | (CLAMP(g, 0, 7) << 3) | CLAMP(b, 0, 3);
}