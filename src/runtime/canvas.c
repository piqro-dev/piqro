#include <runtime/canvas.h>

RT_Canvas canvas_make(Arena* arena, uint16_t width, uint16_t height)
{
	RT_Canvas c = {};

	c.width = width;
	c.height = height;
	c.buffer = arena_push_array(arena, uint8_t, c.width * c.height);

	return c;
}

void canvas_clear(RT_Canvas* c, uint8_t color)
{
	__builtin_memset(c->buffer, color, c->width * c->height);
}

void canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
	x = CLAMP(x, 0, c->width);
	y = CLAMP(y, 0, c->height);

	w = CLAMP(w, 0, c->width);
	h = CLAMP(h, 0, c->height);

	for (; x < w; x++) 
	{
		for (; y < h; y++)
		{
			c->buffer[x + y * c->width] = color;
		}
	}
}

void canvas_put(RT_Canvas* c, int16_t x, int16_t y, uint8_t color)
{
	x = CLAMP(x, 0, c->width);
	y = CLAMP(y, 0, c->height);

	c->buffer[x + y * c->width] = color;
}