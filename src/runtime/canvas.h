#pragma once

#include <base/common.h>
#include <base/arena.h>
#include <base/string.h>

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

	uint16_t line_width;
};

RT_Canvas rt_canvas_make(Arena* arena, uint16_t width, uint16_t height);

void rt_canvas_clear(RT_Canvas* c);

void rt_canvas_present(RT_Canvas* c);

void rt_canvas_line(RT_Canvas* c, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

void rt_canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h);

void rt_canvas_fill_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h);

void rt_canvas_put(RT_Canvas* c, int16_t x, int16_t y);

void rt_canvas_circle(RT_Canvas* c, int16_t cx, int16_t cy, int16_t r);

void rt_canvas_fill_circle(RT_Canvas* c, int16_t cx, int16_t cy, int16_t r);

void rt_canvas_text(RT_Canvas* c, int16_t x, int16_t y, int16_t s, String text);

// we are doing one byte per pixel, 3 bits for R, 3 bits for G, 2 for B.
// R3G3B2
uint8_t rt_canvas_pack_color(uint8_t r, uint8_t g, uint8_t b);