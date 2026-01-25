#pragma once

#include <base/common.h>
#include <base/arena.h>

#include <runtime/config.h>

typedef struct RT_Canvas RT_Canvas;
struct RT_Canvas
{
	uint8_t* back_buffer;
	uint8_t* frame_buffer;

	uint16_t width;
	uint16_t height;

	uint8_t back_color;
	uint8_t fore_color;
};

RT_Canvas rt_canvas_make(Arena* arena, uint16_t width, uint16_t height);

void rt_canvas_clear(RT_Canvas* c);

void rt_canvas_present(RT_Canvas* c);

void rt_canvas_line(RT_Canvas* c, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

void rt_canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h);

void rt_canvas_put(RT_Canvas* c, int16_t x, int16_t y);