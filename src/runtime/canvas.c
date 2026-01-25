#include <runtime/canvas.h>

RT_Canvas rt_canvas_make(Arena* arena, uint16_t width, uint16_t height)
{
	RT_Canvas c = {};

	c.width = width;
	c.height = height;

	c.back_buffer = arena_push_array(arena, uint8_t, c.width * c.height);
	c.fore_buffer = arena_push_array(arena, uint8_t, c.width * c.height);

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
	c->frame_idx ^= 1;
	SWAP(c->fore_buffer, c->back_buffer);
}

void rt_canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h)
{
	for (int16_t i = x; i < x + w; i++) 
	{
		for (int16_t j = y; j < y + h; j++) 
		{
			if (i >= 0 && j >= 0 && i < c->width && j < c->height)
			{
				c->back_buffer[i + j * c->width] = c->fore_color;
			}
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