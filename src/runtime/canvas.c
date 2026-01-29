#include <runtime/canvas.h>

RT_Canvas rt_canvas_make(Arena* arena, uint16_t width, uint16_t height)
{
	RT_Canvas c = {};

	c.width = width;
	c.height = height;

	c.back_buffer = arena_push_array(arena, uint8_t, c.width * c.height);
	c.frame_buffer = arena_push_array(arena, uint8_t, c.width * c.height);

	c.back_color = 0x00;
	c.fore_color = 0xff;

	rt_canvas_clear(&c);

	return c;
}

void rt_canvas_clear(RT_Canvas* c)
{
	__builtin_memset(c->back_buffer, c->back_color, c->width * c->height);
}

void rt_canvas_present(RT_Canvas* c)
{
	__builtin_memcpy(c->frame_buffer, c->back_buffer, c->width * c->height);
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
	for (int16_t i = x; i < x + w; i++) 
	{
		for (int16_t j = y; j < y + h; j++) 
		{
			rt_canvas_put(c, i, j);
		}
	}
}

void rt_canvas_put(RT_Canvas* c, int16_t x, int16_t y)
{
	if (x >= 0 && y >= 0 && x < c->width && y < c->height)
	{
		c->back_buffer[x + y * c->width] = c->fore_color;
	}
}

uint8_t rt_canvas_pack_color(uint8_t r, uint8_t g, uint8_t b)
{
	return (r << 5) | (g << 3) | b;
}